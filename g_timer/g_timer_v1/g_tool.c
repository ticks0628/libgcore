#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "g_tool.h"

int nsleep( int sec,int nano){
	struct timespec req, rem;
	req.tv_sec = sec; 
	req.tv_nsec = nano; 
	while(nanosleep(&req, &rem) == -1) {
		if(errno == EINTR) {
			req = rem;
		//	printf("Sleep interrupted. Remaining time: %ld.%09ld seconds\n", rem.tv_sec, rem.tv_nsec);
		}
		else{
			perror("nanosleep");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

long long gTimeMask[ TIME_MASK_MAX][7]={0};

long long timeslot_get_offset(time_t t){
	struct tm	ptr;
	long long iTimeSlot;

	localtime_r(&t, &ptr);
	t = (ptr.tm_hour*2) + (ptr.tm_min/30);
	iTimeSlot = EV_SCHEDULE_INITIAL_SPOT >> t;
	//printf("%s iTimeSlot=[%llx]\n", __FUNCTION__, iTimeSlot);
	return iTimeSlot;
}

int g_time_mask_init(){
	memset( gTimeMask, 0, sizeof(long long) * TIME_MASK_MAX* 7 );
	return 0;
}
int g_time_mask_set( int type, long long *daySlot){
	//DaySlot Mask initial  48bit = 6 byte = 12xf
	//gEventTimeMask[ type ][2]= strtoll("ffff2000ffff", '\0', 16);
	memcpy( gTimeMask, daySlot, sizeof(long long)*7 );
	return 0;
}
int g_time_mask_chk(int type ){
	time_t now_t = time(NULL);
	long long nowOffset = timeslot_get_offset(now_t), daySlot;
	struct tm now_tm;

	if( type > TIME_MASK_MAX ){
		printf("%s: type exceed max[%d] num=%d\n", __FUNCTION__, TIME_MASK_MAX, type);
		return 0;
	}
	localtime_r( &now_t, &now_tm);
	daySlot = gTimeMask[ type ][ now_tm.tm_wday ];

	if( nowOffset > 0 ){
		if( timeslot_check( daySlot, nowOffset) )
			return 1;
		else	return 0;
	}
	return 0;
}

void convert_time_string(time_t t, char *buf, int len){
	struct tm *timeInfo;
	timeInfo = localtime(&t);
	strftime( buf, len, "%y/%m/%d %H:%M:%S", timeInfo);
}

int lltostr(long long n, char *buf){
  long long c, k;
  memset( buf, 0, sizeof(64) );	

  for (c = 47; c >= 0; c--){
    k = n >> c;

    if (k & 1)
      *buf='1';
    else
      *buf='0';

    buf++;
  }
  return 0;
}
