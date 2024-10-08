#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ev.h"
#include "ev_schedule.h"

int gEventAlive = 0; //0=1st, 1=running, -1=reopen
struct JOB_STATUS jobStatus;
struct EV_RULE ev[ TRIGGER_MAX];
long long gEventTimeMask[ TRIGGER_MAX ][7];

//fn=/tmp/event/yyyymmdd-hhmmss-999.mp4
int fn_convert(struct timeval *tv, char *fn, char *ext){
	struct tm t;
	localtime_r( &tv->tv_sec, &t);
	sprintf( fn, "/tmp/event/%4d%.2d%.2d-%.2d%.2d%.2d-%.3ld%s",
		t.tm_year+1900, t.tm_mon+1, t.tm_mday,
		t.tm_hour, t.tm_min, t.tm_sec, tv->tv_usec/1000, ext);

	return 0;
}

void remove_tmp_file(struct JOB_STRUCT *j ){
	char fn[48]="aaa";
	if( jobStatus.iRamJpgCounter){
		jobStatus.iRamJpgCounter--;
		EV_PRINT("%s: remove before\n", __FUNCTION__ );
		fn_convert( &j->tvJpg, fn, ".jpg");
		EV_PRINT("%s: remove %s\n", __FUNCTION__, fn );
		unlink(fn);
	}
	if( jobStatus.iRamMp4Counter){
		jobStatus.iRamMp4Counter--;
		fn_convert( &j->tvMp4, fn, ".mp4");
		EV_PRINT("%s: remove %s\n", __FUNCTION__, fn );
		unlink(fn);
	};
}

int get_wday(time_t t){
	struct tm	ptr;
	long long iTimeSlot;

	localtime_r(&t, &ptr);
	return ptr.tm_wday;
}

long long timeslot_str_to_hex(char *str){
	int hex;
	hex=strtoll(str, '\0', 16);

	return hex;
}

long long timeslot_get_offset(time_t t){
	struct tm	ptr;
	long long iTimeSlot;

	localtime_r(&t, &ptr);
	t = (ptr.tm_hour*2) + (ptr.tm_min/30);
	iTimeSlot = EV_SCHEDULE_INITIAL_SPOT >> t;
	//printf("%s iTimeSlot=[%llx]\n", __FUNCTION__, iTimeSlot);
	return iTimeSlot;
}


int checkExit(struct JOB_STRUCT *j ){
	if( gEventAlive == 1 ) return 0;
	else if( gEventAlive == 0 ){ //1st boot
		EV_PRINT("%s:Enter alive[%d] type[%d] bRecordStarted[%d] iRunningJobs[%d]\n", 
			__FUNCTION__, gEventAlive, j->iTriggerType, j->bRecordStarted, jobStatus.iRunningJobs);
		event_init();
		return 0;
	}
	else if( gEventAlive== -1 ){ //during restart
		EV_PRINT("%s:Enter alive[%d] type[%d] bRecordStarted[%d] iRunningJobs[%d]\n", 
			__FUNCTION__, gEventAlive, j->iTriggerType, j->bRecordStarted, jobStatus.iRunningJobs);
		if( j->bRecordStarted ){
			EV_PRINT("%s type[%d] bRecordStarted[%d] get restart signal leaving\n", __FUNCTION__, j->iTriggerType, j->bRecordStarted);
			//mq_send( j );
			//mq_recv( j ); //useless, just make sure rec close finished
		}
		return 1;
	}
	return 0;
}

void next_job( struct JOB_STRUCT *j){
	if( checkExit( j ) )   return;

	struct EV_RULE *e=&ev[j->iTriggerType ];
	if ( j->iActCnt != 0){
		int cur = e->iActCount - j->iActCnt;
		j->iActCnt--;
		FP runJob = e->actFunc[ cur ];
		EV_PRINT("%s type[%d][%d] start\n", __FUNCTION__, j->iTriggerType, cur);
		runJob( j );
		EV_PRINT("%s type[%d][%d] stop\n", __FUNCTION__, j->iTriggerType, cur);
	}
	else{ //end of recursion
		remove_tmp_file( j );
		jobStatus.iRunningJobs--;
		EV_PRINT("%s type[%d] end and leaving\n", __FUNCTION__, j->iTriggerType);
	}
}

void *_push_job(void* arg ){
	long long type = (long long) arg, ret;
	struct JOB_STRUCT job, *j=&job;
	struct EV_RULE *e=&ev[ type ] ;
	if( checkExit( j ) ){
		remove_tmp_file( j );
		if( jobStatus.iRunningJobs == 0)
	                event_init();
		else	return (void*)1;
	}

	if( ev[ type ].iActCount == 0 ) return (void*)1;

	time_t now = time(NULL);
	long long ts = timeslot_get_offset( now );
	ret = timeslot_check( gEventTimeMask[ type ][ get_wday(now) ], ts);
	EV_PRINT("%s: %s type[%lld] wday=%d, mask[0x%llx] ts=0x%llx\n", __FUNCTION__, (ret)?"Match": "Not Match", type,
			get_wday(now), gEventTimeMask[ type ][ get_wday(now) ], ts);
	if( ret == 0){
		return (void*)1;
	}

	memset( j, 0, sizeof(struct JOB_STRUCT));
	EV_PRINT("%s type=%lld launch!\n", __FUNCTION__, type);
	if( now - e->tLastRun > e->tTriggerCooldownPeriod){
		j->iTriggerType = type;
		j->iActCnt =  e->iActCount;
		j->bSdMp4 = e->bSdMp4;
		j->bSdJpg = e->bSdJpg;
		e->tLastRun = now;
		jobStatus.iRunningJobs++;
		next_job( &job );
	}
	return (void*)0;
}

struct FUNC_TABLE actionTable[3]={
	{RAM_JPG, ram_jpg},
	{RAM_MP4, ram_mp4},
	{FTP, ftp},
};

int event_stop(){
	int	i;
	gEventAlive = -1;
	sche_stop();
	EV_PRINT("%s!!! trigger!!\n", __FUNCTION__);
	for(i=0; i<TRIGGER_MAX; i++){
		if( ev[i].iActCount && ev[i].pt ){
			pthread_cancel(ev[i].pt);
		}	
	}
	event_init();
}

int event_init(){
	struct EV_RULE *e;
	int i=0;

	//UTC+0 Must enable!!!!!
	setenv("TZ", "", 0);
	tzset();
//event
	printf("%s! start\n", __FUNCTION__);
	memset( ev, 0, sizeof(struct EV_RULE)*TRIGGER_MAX );
	memset( &jobStatus, 0, sizeof(struct JOB_STATUS));

	//DaySlot Mask initial 	48bit = 6 byte = 12xf
	gEventTimeMask[ TRIGGER_MOTION ][2]= strtoll("ffff2000ffff", '\0', 16);

	//motion scan event rule and fill it
	e = &ev[ TRIGGER_MOTION ];
	e->actFunc[ i++] = actionTable[ RAM_JPG ].fp;
	e->actFunc[ i++] = actionTable[ RAM_MP4 ].fp;
	e->actFunc[ i++] = actionTable[ FTP].fp;
	e->iActCount = i;
	e->bSdJpg =1;
	e->bSdMp4 =1;
	e->tTriggerCooldownPeriod = 3;
	e->pt = 0;

	i=0; e = &ev[ TRIGGER_MANUAL ];
	e->actFunc[ i++] = actionTable[ RAM_JPG ].fp;

	//Must, start event
	gEventAlive = 1;

	//schedule related
	sche_init();
}
