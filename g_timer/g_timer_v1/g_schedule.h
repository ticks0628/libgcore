#ifndef __EV_SCHEDULE_H__
#define __EV_SCHEDULE_H__
#include <g_tool.h>


#define GSCHEDULE_STRUCT_MAX 10

struct GSCHEDULE_TIME{
	time_t		start_t;
	time_t		end_t;
};

struct GSCHEDULE_STRUCT{
	intptr_t	type;
	char		enabled;
	char		bRunning;
	FP		func;           //call back function
	char		name[MAX_NAME];
	long long	daySlot[7];
	struct GSCHEDULE_TIME	t;
};


void g_schedule_init();
int g_schedule_stop(intptr_t type);
int g_schedule_start(intptr_t type);
void gschedule_set(intptr_t type, long long *daySlot, char *name, FP func);
#endif
