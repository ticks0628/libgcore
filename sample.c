#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include "ev.h"
#include "ev_schedule.h"


int motion_detection(){
	while(1){

		push_job( TRIGGER_MOTION);
		sleep(5);
	//	event_stop();
	}
}


	
int main(){
	motion_detection();
	
	sche_func(TIME_SCHE_SD);
	sleep(10);
	
}
