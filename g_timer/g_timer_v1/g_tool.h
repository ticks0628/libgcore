#ifndef __G_TOOL_H__
#define __G_TOOL_H__
typedef void* (*FP)(void *arg);
int nsleep( int sec,int nano);

#define EV_SCHEDULE_INITIAL_SPOT 0b100000000000000000000000000000000000000000000000
#define timeslot_check( slot, off ) ((slot & off) == off) ?1:0

//Time Mask related function
#define TIME_MASK_MAX 20
long long timeslot_get_offset(time_t t);
int g_time_mask_init();
int g_time_mask_set( int type, long long *daySlot);
int g_time_mask_chk(int type );


void convert_time_string(time_t t, char *buf, int len);
int lltostr(long long n, char *buf);
#endif
