#ifndef __EV_H__
#define __EV_H__
#include <sys/time.h>
#include <pthread.h>

#define ACTION_MAX 10
#define RAM_MP4_MAX 3
#define RAM_JPG_MAX 3

struct JOB_STRUCT{
	int iTriggerType;
	int iActCnt;
	struct timeval tvJpg, tvMp4;
	int bSdMp4, bSdJpg, bRecordStarted;
};

typedef void (*FP)(struct JOB_STRUCT*);
struct FUNC_TABLE{
	int	type;
	FP	fp;
};

enum ACTION_TYPE{
	RAM_JPG=0,
	RAM_MP4,
	FTP,
	EMAIL,
	GPOUT,
};

enum TRIGGER_TYPE{
	TRIGGER_MANUAL =0,
	TRIGGER_MOTION,
	TRIGGER_GPIN,
	TRIGGER_AI,
	TRIGGER_SD_ERROR,
	TRIGGER_SD_NOCARD,
	TRIGGER_REBOOT,
	TRIGGER_MAX
};

struct JOB_STATUS{
	int iRamJpgCounter;
	int iRamMp4Counter;
	int iRunningJobs;
};

struct EV_RULE{
	FP	actFunc[ACTION_MAX];
	int	tTriggerCooldownPeriod;
	int	tLastRun;
	int	iActCount;
	int	bSdMp4:1;
	int	bSdJpg:1;
	pthread_t	pt;
};

extern struct JOB_STATUS jobStatus;
extern struct EV_RULE ev[ TRIGGER_MAX];
extern int gEventAlive; //0=1st, 1=running, -1=reopen
			  
void next_job( struct JOB_STRUCT *j);
void *_push_job(void *arg );

//external
#define EV_PRINT printf
#define EV_SCHEDULE_INITIAL_SPOT 0b100000000000000000000000000000000000000000000000
//#define push_job( type )  _push_job( type );
#define push_job( type )  pthread_create( &ev[type].pt, NULL, _push_job, (void*)type);
#define timeslot_check( slot, off ) ((slot & off) == off) ?1:0

int event_init();
int event_stop();

void ram_jpg(struct JOB_STRUCT *j);
void ram_mp4(struct JOB_STRUCT *j);
void ftp(struct JOB_STRUCT *j);

long long timeslot_get_offset(time_t t);
long long timeslot_str_to_hex(char *str);
#endif

