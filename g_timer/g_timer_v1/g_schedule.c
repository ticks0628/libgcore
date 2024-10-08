#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "g_timer.h"
#include "g_schedule.h"

/*
struct PTHREAD_STRUCT ptStruct[ TIME_SCHE_MAX ]={
	{TIME_SCHE_SD, "sche_record", sche_record, 0, 0, 0, 0},
	{TIME_SCHE_LIGHT, "sche_light", sche_light, 0, 0, 0, 0},
};
*/

//for schedule
#define TOMORROW_ZERO_T()  today_zero_t()+86400
time_t today_zero_t(){
	struct tm ptr;
	time_t t;
	t = time(NULL);
	localtime_r(&t, &ptr);
	ptr.tm_hour = 0;
	ptr.tm_min = 0;
	ptr.tm_sec = 0;
	t = mktime(&ptr);
	return t;
}

int timeslot_get_start_end(struct GSCHEDULE_STRUCT *p, long long wday, long long offset_cur){
	int i=0, j=0;
	time_t tZero = today_zero_t();
	long long offset = offset_cur;
	while( EV_SCHEDULE_INITIAL_SPOT != offset ){
		//printf("%lx %llx\n", EV_SCHEDULE_INITIAL_SPOT, offset);
		offset<<=1;
		i++;
	}
	//p->t.start_t = (i == 0 ) ? tZero: tZero + (i)*1800;
	p->t.start_t = tZero + i * 1800;
	//find end
	offset = offset_cur;
	for(j=i; j< 48; j++){
		offset>>=1;
		//printf("wday=%llx, offset=%llx and=%llx\n", wday, offset, wday&offset);
		if( (wday & offset) == 0 ) break;
	}
	p->t.end_t = tZero + j * 1800;
	
	return i;
}
// find location and return time_t of S and E (from MSB to LSB)
// N=now, S=start, E=end, tsoffset search from A to B
// 0000000000000011111000000000
// A00000000N0000S111E00000000B

int timeslot_get_next_start_end_t( struct GSCHEDULE_STRUCT *p ){
	time_t now_t = time(NULL), asap=0;
	long long nowOffset = timeslot_get_offset(now_t), daySlot;
	struct tm now_tm;

	localtime_r( &now_t, &now_tm);
	daySlot = p->daySlot[ now_tm.tm_wday ];
	
	if( timeslot_check( daySlot, nowOffset) ){
		asap=1;
	}
	while( nowOffset > 0 ){
		if( timeslot_check( daySlot, nowOffset) ){
			timeslot_get_start_end( p, daySlot, nowOffset);
			if( asap ){
				p->t.start_t = now_t;
			}
			printf("%s: Matched! dayslot[%llx] nowOffset_t[%llx] duration[%ld] e[%ld] s[%ld]\n", __FUNCTION__, daySlot, nowOffset, p->t.end_t - p->t.start_t, p->t.end_t, p->t.start_t);
			return 1;
		}
		nowOffset>>=1;
	}
	//No schedule left today, check it in tomorrow zero
	p->t.start_t = TOMORROW_ZERO_T();
	p->t.end_t   = 0;
	return 0;
}
struct GSCHEDULE_STRUCT gScheduleStruct[ GSCHEDULE_STRUCT_MAX ];

void debug_info(struct GSCHEDULE_STRUCT *p, time_t now, int ret){
	char		startStr[20]="", endStr[20]="", nowStr[20]="", slot[65]="";
	long long	daySlot;
	struct tm	now_tm;
	long long nowOffset = timeslot_get_offset(now);

	localtime_r( &now, &now_tm);
	daySlot = p->daySlot[ now_tm.tm_wday ];
	lltostr(daySlot, slot);

	convert_time_string( p->t.start_t, startStr, 20 );
	convert_time_string( p->t.end_t, endStr, 20 );
	convert_time_string( now, nowStr, 20 );

	printf("%s: slot[%llx]:%s week=%d\n", __FUNCTION__, daySlot, slot, now_tm.tm_wday );
	lltostr(nowOffset, slot);
	printf("%s: now:%s week=%d\n", __FUNCTION__, slot, now_tm.tm_wday );
	printf("%s: now:%s %ld\n", __FUNCTION__, nowStr, now);
	printf("%s: sta:%s %ld\n", __FUNCTION__, startStr, p->t.start_t);
	printf("%s: end:%s %ld\n", __FUNCTION__, endStr, p->t.end_t);
	printf("%s:%s: ret=%s, during=%ld\n", __FUNCTION__, p->name, (ret)?"found": "not found", p->t.end_t-p->t.start_t );
}
void *schedule_handler(void *arg){
	intptr_t which = (intptr_t) arg;
	struct GTIMER_STRUCT	*ts = &timerStruct[which];
	struct GSCHEDULE_STRUCT *p = (struct GSCHEDULE_STRUCT*)ts->arg;

	time_t now = time(NULL);
	printf("%s: time up! call[%s] \n", __FUNCTION__, p->name);
	debug_info( p, now, 0);

	//run
	p->bRunning = 1;
	p->func( (void*) &p->t );
	return NULL;
}

void gschedule_set(intptr_t type, long long *daySlot, char *name, FP func){
	struct GSCHEDULE_STRUCT *p = &gScheduleStruct[ type ];

	p->type = type;
	p->enabled = 1;
	p->func = func;
	snprintf( p->name, MAX_NAME, "%s", name);
	memcpy( p->daySlot, daySlot, sizeof(long long)*7);

	g_schedule_start( type );
}

void g_schedule_start_single(intptr_t type){
	struct GSCHEDULE_STRUCT *p=&gScheduleStruct[ type ];
	int		ret;
	time_t 		now = time(NULL), sleepSec=0;

	ret = timeslot_get_next_start_end_t( p );
	debug_info( p, now, ret);

	sleepSec =  p->t.start_t - now ;
	printf("%s: sleepSec=%ld now[%ld], start[%ld]\n", __FUNCTION__, sleepSec, now, p->t.start_t);
	if( sleepSec <1  ) //find 1)next schedule 2)tomorrow_zero_t
		g_timer_set(type, 0, 1000000, p->name, schedule_handler, (void*)type, 0);
	else
		g_timer_set(type, sleepSec, 0, p->name, schedule_handler, (void*)type, 0);
}
int g_schedule_start(intptr_t type){
	if(type != -1 && gScheduleStruct[ type ].enabled){
		g_schedule_start_single( type );
	}
	else{
		for(int i=0; i<GSCHEDULE_STRUCT_MAX; i++){
			if(gScheduleStruct[ i ].enabled){
				g_schedule_start_single( i );
			}
		}
	}
	return 0;
}
void g_schedule_stop_single(intptr_t type){
	struct GSCHEDULE_STRUCT *p=&gScheduleStruct[ type ];
		if( p->bRunning ){
			//todo: kill thread
			p->bRunning = 0;
		}
		else{
			g_timer_stop(type);
		}
}
int g_schedule_stop(intptr_t type){

	if(type != -1 && gScheduleStruct[ type ].enabled){
		g_schedule_stop_single( type );
	}
	else{
		for(int i=0; i<GSCHEDULE_STRUCT_MAX; i++){
			if(gScheduleStruct[ i ].enabled){
				g_schedule_stop_single( i );
			}
		}

	}
	return 0;
}

void g_schedule_init(){
	memset(&gScheduleStruct, 0, sizeof(struct GSCHEDULE_STRUCT) * GSCHEDULE_STRUCT_MAX );

}
