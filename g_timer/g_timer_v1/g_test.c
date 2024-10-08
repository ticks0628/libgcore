#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include <g_tool.h>

#include "g_timer.h"
#include "g_schedule.h"

enum ENUM_TYPE{
	eCHK_5G,
	eCHK_LED
};

void *f5g(void *arg){
	intptr_t which = (intptr_t) arg;
	struct GTIMER_STRUCT	*ts = &timerStruct[which];
	printf("%s: fire\n", ts->name);
	return NULL;
}

void *led(void *arg){
	intptr_t which = (intptr_t) arg;
	struct GTIMER_STRUCT	*ts = &timerStruct[which];
	printf("%s: fire\n", ts->name);
	return NULL;
}

void *timer_test(void *arg){
	g_timer_init(NULL);

	g_timer_set(eCHK_5G, 1, 0, "i'm chk 5g", f5g, NULL, 1);
	g_timer_set(eCHK_LED, 1, 0, "i'm led", led, NULL, 1);

	nsleep(3,0);
	g_timer_stop(-1);
	nsleep(3,0);
	g_timer_start(-1);

	return NULL;
}

void *sche_record(void *arg){
	struct GSCHEDULE_STRUCT *p = (struct GSCHEDULE_STRUCT*) arg;
	printf("%s launch! start_t=%ld end_t=%ld\n", __FUNCTION__, p->t.start_t, p->t.end_t);
	sleep(10);
	return NULL;
}
void *sche_light(void *arg){
	struct GSCHEDULE_STRUCT *p = (struct GSCHEDULE_STRUCT*) arg;
	printf("%s launch! start_t=%ld end_t=%ld\n", __FUNCTION__, p->t.start_t, p->t.end_t);
	sleep(10);
	return NULL;
}
enum TIME_SCHEDULE_ENUM{
	TIME_SCHE_SD=0,
	TIME_SCHE_LIGHT,
	TIME_SCHE_MAX,
};

void *schedule_test(void *arg){
	//struct GSCHEDULE_STRUCT *p;
	long long daySlot[7]={0};


	daySlot[0] = strtoll("ffff2000ffff", '\0', 16);
	daySlot[1] = 0x603e0;
	daySlot[2] = 0x603e0;
	daySlot[3] = 0x603e0;
	daySlot[4] = 0x603e0;
	daySlot[5] = 0x4603e0;
	daySlot[6] = 0x603e0;
	gschedule_set( TIME_SCHE_SD, daySlot, "schedule SD", sche_record);


	daySlot[0] = 0x603e0;
	daySlot[1] = 0x603e0;
	daySlot[2] = 0x603e0;
	daySlot[3] = 0x603e0;
	daySlot[4] = 0x603e0;
	daySlot[5] = 0x2603e0;
	daySlot[6] = 0x603e0;
	gschedule_set( TIME_SCHE_LIGHT, daySlot, "schedule light", sche_light);
	
	return NULL;
}
void *mask_test(void *arg){
	long long daySlot[7]={0};
	int	ret;

	daySlot[0] = 0x603e0;
	daySlot[1] = 0x603e0;
	daySlot[2] = 0x603e0;
	daySlot[3] = 0x603e0;
	daySlot[4] = 0x603e0;
	daySlot[5] = 0x603e0;
	daySlot[6] = 0x603e0;
	g_time_mask_set( TIME_SCHE_SD, daySlot);
	ret = g_time_mask_chk( TIME_SCHE_SD );

	printf("%s: ret[%d]\n", __FUNCTION__, ret );
	return NULL;
}
int main(){
	//pthread_t pt_timer;
	//pthread_create( &pt_timer, NULL, timer_test, NULL);
	
	g_timer_init(NULL);
	g_schedule_init();
	g_time_mask_init();
	mask_test(NULL);

	pthread_t pt_schedule;
	pthread_create( &pt_schedule, NULL, schedule_test, NULL);

	while(1){
		printf("sleep5\n");
		nsleep(20, 0);
	}
	return 0;
}
