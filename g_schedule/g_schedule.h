#ifndef __EV_SCHEDULE_H__
#define __EV_SCHEDULE_H__

enum TIME_SCHEDULE_ENUM{
	TIME_SCHE_SD=0,
	TIME_SCHE_LIGHT,
	TIME_SCHE_MAX,
};

struct GSCHEDULE_STRUCT{
	int		type;
	int		enabled;
	FP		func;           //call back function
	char		name[16];
	long long	daySlot[7];
	time_t		start_t;
	time_t		end_t;
};


void g_schedule_init();
int g_schedule_stop(int type);
int g_schedule_start(int type);
void gschedule_set(int type, int daySlot[7], char *name, FP func);
#endif
