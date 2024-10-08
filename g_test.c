#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include <g_timer.h>
#include "g_tool.h"

#include <threadpool.h>

enum ENUM_TYPE{
	eCHK_5G,
	eCHK_LED
};

threadpool_t    pool;

void *f5g(void *arg){
	struct GTIMER_STRUCT *ts = (struct GTIMER_STRUCT*)arg;
	printf("%s: fire\n", ts->name);

	return NULL;
}


void *led(void *arg){
	struct GTIMER_STRUCT *ts = (struct GTIMER_STRUCT*)arg;
	printf("%s: fire\n", ts->name);
	return NULL;
}

void timer_handler( int signo, siginfo_t *si, void *uc ){
	struct GTIMER_STRUCT    *ts = (struct GTIMER_STRUCT*) si->si_value.sival_ptr;
        int             ret;

	//printf("FUNC:%16s: signo=%d sival=%ld called!\n", ts->name, signo, (long)(ts->id));
	if( signo != GSIGNO_USR2 ) return;
	if( si->si_overrun){
		fprintf(stderr, "%s)Timer[%s] overrun[%d]/%d!!!!\n", __func__, ts->name, si->si_overrun, ts->overRunCnt);
		ts->overRunCnt++;
	}
	ret=threadpool_add(&pool, ts->func, ts, 1);
	if(ret)	printf("%s: error!\n",__FUNCTION__);
}


void *timer_test(void *arg){
	g_timer_init(timer_handler);

	g_timer_set(eCHK_5G, 1, 0, "i'm chk 5g", f5g, 1);
	g_timer_set(eCHK_LED, 1, 0, "i'm led", led, 1);

	nsleep(3,0);
	g_timer_stop(-1);
	nsleep(3,0);
	g_timer_start(-1);

	return NULL;
}

int main(){
	pthread_t pt;

	pthread_create( &pt, NULL, timer_test, NULL);

	threadpool_create(&pool, 10, 20);

	while(1){
		printf("sleep5\n");
		nsleep(5, 0);
	}
	return 0;
}
