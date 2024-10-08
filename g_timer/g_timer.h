#ifndef __G_TIMER_H__
#define __G_TIMER_H__
#include <signal.h>
#include <g_tool.h>

typedef void (*TIMER_HANDLER)(int, siginfo_t *, void *);

#define MAX_NAME 16
struct GTIMER_STRUCT{
	timer_t		id;
	char		enabled;
	int		overRunCnt;
	struct itimerspec its;
	FP		func;		//call back function
	void*		arg;		//call back argument
	char		name[MAX_NAME];
};
#define GSIGNO_USR2  SIGUSR2 //SIGRTMIN;
#define GTIMER_MAX 20

int g_timer_init();
int g_timer_stop(int num);
int g_timer_start(int num);
int g_timer_set( int num, time_t sec, suseconds_t, char *name, FP func, void *arg, int repeat);
extern struct GTIMER_STRUCT timerStruct[ GTIMER_MAX ];
#endif
