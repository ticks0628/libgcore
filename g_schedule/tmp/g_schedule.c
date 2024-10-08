#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "ev.h"
#include "ev_schedule.h"

pthread_mutex_t	gMutex;
pthread_cond_t	gCond;

struct PTHREAD_STRUCT ptStruct[ TIME_SCHE_MAX ]={
	{TIME_SCHE_SD, "sche_record", sche_record, 0, 0, 0, 0},
	{TIME_SCHE_LIGHT, "sche_light", sche_light, 0, 0, 0, 0},
};

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

#ifndef __RECORD_H__
void *sche_record(int start_t, int end_t){
	printf("%s launch!\n", __FUNCTION__);
	sleep(10);
}
#endif
#ifndef __LIGHT_H__
void *sche_light(int start_t, int end_t){
	printf("%s launch!\n", __FUNCTION__);
	sleep(10);
}
#endif


int timeslot_get_start_end(struct PTHREAD_STRUCT *p, long long wday, long long offset_cur){
	int i=0, j=0;
	time_t tZero = today_zero_t();
	long long offset = offset_cur;
	while( EV_SCHEDULE_INITIAL_SPOT != offset ){
		//printf("%lx %llx\n", EV_SCHEDULE_INITIAL_SPOT, offset);
		offset<<=1;
		i++;
	}
	p->start_t = (i == 0 ) ? tZero: tZero + (i-1)*1800;
	//find end
	offset = offset_cur;
	for(j=i; j< 48; j++){
		offset>>=1;
		printf("wday=%llx, offset=%llx and=%llx\n", wday, offset, wday&offset);
		if( (wday & offset) == 0 ) break;
	}
	p->end_t = tZero + j * 1800;
	
	return i;
}
// find location and return time_t of S and E (from MSB to LSB)
// N=now, S=start, E=end, tsoffset search from A to B
// 0000000000000011111000000000
// A00000000N0000S111E00000000B
int timeslot_get_next_start_end_t( struct PTHREAD_STRUCT *p ){
	time_t now_t = time(NULL);
	long long tsOffset = timeslot_get_offset(now_t), daySlot;
	struct tm now_tm;

	localtime_r( &now_t, &now_tm);
	daySlot = p->daySlot[ now_tm.tm_wday ];

	while( tsOffset > 0 ){
		if( timeslot_check( daySlot, tsOffset) ){
			timeslot_get_start_end( p, daySlot, tsOffset);
			printf("%s: Matched! dayslot[%llx] offset_t[%llx] duration[%ld]\n", __FUNCTION__, daySlot, tsOffset, p->end_t - p->start_t);
			return 0;
		}
		tsOffset>>=1;
	}
	//No schedule left today, check it in tomorrow zero
	p->start_t = TOMORROW_ZERO_T();
	p->end_t   = 0;
	return 0;
}

pthread_once_t	once = PTHREAD_ONCE_INIT;
void *sche_func(void *arg){
	long long	type = (long long) arg;
	struct timespec	timeout;
	int		ret;
	time_t 		now = time(NULL);
	pthread_once( &once, sche_init);

	struct PTHREAD_STRUCT *p = &ptStruct[ type ];

	printf("%s->%s start\n", __FUNCTION__, p->name);

	while(gEventAlive){
		//ret = timeslot_get_next_start_end_t( TIME_SCHE_SD, &start_t, &end_t);
		ret = timeslot_get_next_start_end_t( p );
		printf("%s:%s: ret=%s start_t=%ld end_t=%ld now=%ld, during=%ld\n", __FUNCTION__, p->name, (ret)?"not found": "found", 
			p->start_t, p->end_t, time(NULL), p->end_t-p->start_t);

		if( p->start_t > now ){ //blocking until 1)next schedule 2)tomorrow_zero_t
			timeout.tv_sec = p->start_t;
			timeout.tv_nsec = 0;
			printf("%s:%s: timedwait now=%ld timeout=%ld\n", __FUNCTION__, p->name, now, timeout.tv_sec);
	
			pthread_mutex_lock(&gMutex );
			ret = pthread_cond_timedwait(&gCond, &gMutex, &timeout);
			pthread_mutex_unlock(&gMutex );
			if( ETIMEDOUT != ret ) break; //signal coming
			printf("%s:%s: timedwait stop ret[%d]\n", __FUNCTION__, p->name, ret);
		}
		else p->ptfunc(p->start_t, p->end_t);
	}
}
int sche_stop(){
	void	*retval=NULL;
	int	i;
	EV_PRINT("%s: enter\n", __FUNCTION__);
	pthread_cond_signal(&gCond);

	for(i =0; i< TIME_SCHE_MAX; i++){
		if (pthread_join(ptStruct[ i ].pt, NULL) != 0) {
			EV_PRINT("%s: pthread_join[%d] fail!!!\n", __FUNCTION__, i);
		}
		else	EV_PRINT("%s: pthread_join[%d] success\n", __FUNCTION__, i);
		if( retval != NULL ) free(retval);
		ptStruct[ i ].pt=0;
	}
	EV_PRINT("%s: leave\n", __FUNCTION__);
	return 0;
}

void sche_init(){
	printf("%s: launch!\n", __FUNCTION__);
	long long type;
	struct PTHREAD_STRUCT *p;
	pthread_mutex_init(&gMutex, NULL);
	pthread_cond_init(&gCond, NULL);

	type = TIME_SCHE_SD;
	p = &ptStruct[ type ];
	p->daySlot[0] = strtoll("ffff2000ffff", '\0', 16);
	p->daySlot[1] = 0x603e0;
	//p->daySlot[2] = 0x603e0;
	p->daySlot[3] = 0x603e0;
	p->daySlot[4] = 0x603e0;
	p->daySlot[5] = 0x603e0;
	p->daySlot[6] = 0x603e0;
	p->pt = 0;
	pthread_create( &p->pt, NULL, sche_func, (void*)type);

	type = TIME_SCHE_LIGHT;
	p = &ptStruct[ type ];
	p->daySlot[0] = 0x603e0;
	p->daySlot[1] = 0x603e0;
	p->daySlot[2] = 0x603e0;
	p->daySlot[3] = 0x603e0;
	p->daySlot[4] = 0x603e0;
	p->daySlot[5] = 0x603e0;
	p->daySlot[6] = 0x603e0;
	p->pt = 0;
	pthread_create( &p->pt, NULL, sche_func, (void*)type);
}

