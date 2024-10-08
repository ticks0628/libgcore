#ifndef __EV_SCHEDULE_H__
#define __EV_SCHEDULE_H__

enum TIME_SCHEDULE_ENUM{
	TIME_SCHE_SD=0,
	TIME_SCHE_LIGHT,
	TIME_SCHE_MAX,
};

typedef void* (*PTFUNC)(int , int );
struct PTHREAD_STRUCT{
	int		type;
	char		name[16];
	PTFUNC		ptfunc;
	long long	daySlot[7];
	time_t		start_t;
	time_t		end_t;

	pthread_t	pt;
};
//Internal

void *sche_record(int start_t, int end_t);
void *sche_light(int start_t, int end_t);

void *sche_func(void *arg);

//Extern 
void sche_init();
int sche_stop();
#endif
