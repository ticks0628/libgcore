#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "g_timer.h"

struct GTIMER_STRUCT timerStruct[ GTIMER_MAX ]={0};

void g_timer_handler( int signo, siginfo_t *si, void *uc ){
	//struct GTIMER_STRUCT	*ts = (struct GTIMER_STRUCT*) si->si_value.sival_ptr;
	intptr_t which = (intptr_t) si->si_value.sival_ptr;
	struct GTIMER_STRUCT	*ts = &timerStruct[which];

	printf("FUNC:%16s: signo=%d sival=%ld called!\n", ts->name, signo, (long)(ts->id));
	if( signo != GSIGNO_USR2 ) return;

	if( si->si_overrun){
		fprintf(stderr, "%16s)Timer[%s] overrun[%d]/%d!!!!\n", __func__, ts->name, si->si_overrun, ts->overRunCnt);
		ts->overRunCnt++;
	}
	ts->func((void*)which);
}

int g_timer_start(int num){
	int			i, ret;
	struct GTIMER_STRUCT	*ts;

	if(num!= -1 ){
		ts=&timerStruct[ num];
		if(ts->enabled){
			timer_settime(ts->id, 0, &ts->its, NULL);
			printf("%s: ret[%d] name=%16s, id=%p\n", __FUNCTION__, ret, ts->name, ts->id);
		}
	}
	else{
		for(i=0; i<GTIMER_MAX; i++){
			ts=&timerStruct[ i ];
			if(ts->enabled){
				//printf("%ld %ld\n", its.it_value.tv_sec, its.it_value.tv_nsec);
				ret = timer_settime(ts->id, 0, &ts->its, NULL);
				printf("%s: ret[%d] name=%16s, id=%p\n", __FUNCTION__, ret, ts->name, ts->id);
			}
		}

	}
	return 0;
}

int g_timer_stop(int num){
	int			i, ret;
	struct itimerspec       its;
	struct GTIMER_STRUCT	*ts;

	memset(&its, 0, sizeof(struct itimerspec));
	
	if(num!= -1 ){
		if(timerStruct[ num].enabled){
			ts=&timerStruct[ num];
			ret =timer_settime(ts->id, 0, &its, NULL);
			printf("%s: ret[%d] name=%16s, id=%p\n", __FUNCTION__, ret, ts->name, ts->id);
		}
	}
	else{
		for(i=0; i<GTIMER_MAX; i++){
			if(timerStruct[ i ].enabled){
				ts=&timerStruct[ i ];
				ret =timer_settime( ts->id, 0, &its, NULL);
				printf("%s: ret[%d] name=%16s, id=%p\n", __FUNCTION__, ret, ts->name, ts->id);
			}
		}

	}
	//printf("%s: num=%d name=%16s, id=%p timer set success[%d]!\n", __FUNCTION__, num, ts->name, ts->id, ret);
	return 0;
}

int g_timer_init(){
	struct sigaction        sa;
	struct sigevent         se;
	struct GTIMER_STRUCT	*ts;
	int			ret;
	
	sa.sa_flags = SA_SIGINFO;
	sa.sa_flags |= SA_RESTART;
	sa.sa_sigaction = g_timer_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(GSIGNO_USR2, &sa, NULL) == -1){
		fprintf(stderr, "%s: Failed to setup signal handling.\n", __FUNCTION__);
		return(-1);
	}

	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = GSIGNO_USR2;
	for(intptr_t i=0; i<GTIMER_MAX; i++){
		ts=&timerStruct[ i ];
		se.sigev_value.sival_ptr = (void*)i;
		ret = timer_create(CLOCK_REALTIME, &se, &ts->id);
		if( ret != 0){
			fprintf(stderr, "%s) fail! ret=%d errno=%d errstr=%s\n", __func__, ret, errno, strerror(errno) );
			exit(-1);
		}
	}

	return 0;
}

int g_timer_set( int num, time_t sec, suseconds_t nsec, char *name, FP func, void *arg, int repeat){
	struct GTIMER_STRUCT	*ts;

	if( func == NULL){
		printf("%s: input NULL func!\n", __FUNCTION__ );
	        return -1;
	}
	if( num > GTIMER_MAX ){
		printf("%s: input timer[%d] undefine!\n", __FUNCTION__, num);
		return -1;
	}
	ts=&timerStruct[ num];
	ts->its.it_value.tv_sec	= sec;
	ts->its.it_value.tv_nsec= nsec;
	ts->enabled= 1;
	ts->func = func;
	ts->arg	 = arg;
	snprintf( ts->name, MAX_NAME, "%s", name );

	//printf("%ld %ld\n", its.it_value.tv_sec, its.it_value.tv_nsec);
	if( repeat ){
		ts->its.it_interval = ts->its.it_value;
	}
	else{
		ts->its.it_interval.tv_sec = 0;
		ts->its.it_interval.tv_nsec= 0;
	}
	g_timer_start(num);
	return 0;
}

