#include <stdio.h>
#include <time.h>

typedef void* (*FP)(void *arg);
#define MAX_NAME 32

struct GTIMER_STRUCT{
	timer_t		id;
	char		enabled;
	int		overRunCnt;
	struct itimerspec its;
	FP		func;		//call back function
	void*		arg;		//call back argument
	char		name[MAX_NAME];
	char		bRepeat;
};
void* fCHK_5G(void *arg){}
void* fCHK_OSD_REFLUSH(void *arg){}


// TIMER Table initialize -----------------------
#define ITIMERSPEC_INIT { {0, 0}, {0, 0} }
#define ITIMERSPEC_1S	{ {1, 0}, {0, 0} }
#define ITIMERSPEC_10S	{ {5, 0}, {0, 0} }

#define TIMER_DATA_LIST \
    X(eCHK_5G,		0, 1, 0, ITIMERSPEC_10S,	fCHK_5G, NULL,		"Check 5G connect", 1) \
    X(eCHK_OSD_REFLUSH,	0, 1, 0, ITIMERSPEC_1S,		fCHK_OSD_REFLUSH, NULL,	"Check osd every sec", 1) \

enum ENUM_DATA {
    #define X( enum_name, id, enabled, overRunCnt, its, func, arg, name, repeat) enum_name,
    TIMER_DATA_LIST
    #undef X
};

struct GTIMER_STRUCT gTimerStruct[] = {
    #define X( enum_name, id, enabled, overRunCnt, its, func, arg, name, repeat) { id, enabled, overRunCnt, its, func, arg, name, repeat},
    TIMER_DATA_LIST
    #undef X
};

#define GTIMER_MAX sizeof(gTimerStruct)
//-------------------------------------

void print_data() {
    for (int i = 0; i < GTIMER_MAX / sizeof(gTimerStruct[0]); ++i) {
        printf("Enum: %d, Name: %s, Value: %d\n", i, gTimerStruct[i].name, gTimerStruct[i].bRepeat);
    }
}

int main() {
    print_data();
        printf("Enum: %d, Name: %s, Value: %d\n", eCHK_OSD_REFLUSH, gTimerStruct[eCHK_OSD_REFLUSH].name, gTimerStruct[eCHK_OSD_REFLUSH].bRepeat);
    return 0;
}

