#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "ev.h"

void snap_shot(struct JOB_STRUCT *j){
	gettimeofday( &j->tvJpg, NULL);
	jobStatus.iRamJpgCounter ++;
	EV_PRINT("%s! type[%d]\n", __FUNCTION__, j->iTriggerType);
	return ;
}
void event_record_start(struct JOB_STRUCT *j ){
	EV_PRINT("%s! type[%d]\n", __FUNCTION__, j->iTriggerType);
	j->bRecordStarted++;
	//mq_send( j);
	//mq_recv( j);
	j->bRecordStarted--;
}

void ram_mp4(struct JOB_STRUCT *j){
	if ( jobStatus.iRamMp4Counter <RAM_MP4_MAX ){
		EV_PRINT("%s! type[%d] start\n", __FUNCTION__, j->iTriggerType);
		gettimeofday( &j->tvMp4, NULL);
		jobStatus.iRamMp4Counter ++;
		event_record_start( j );
	      	//overwrite job->tvMp4 by tv in event_record_start
		//blocking 15s   jobStatus.iRamMp4Counter
	}
	next_job( j);
	EV_PRINT("%s! type[%d] leave\n", __FUNCTION__, j->iTriggerType);
}
void ram_jpg(struct JOB_STRUCT *j){   
	if ( jobStatus.iRamJpgCounter <RAM_JPG_MAX ){
		EV_PRINT("%s! type[%d] start\n", __FUNCTION__, j->iTriggerType);
		jobStatus.iRamJpgCounter ++;
		snap_shot( j );    //overwrite job->tvJpg by tv by snap_shot
	}
	next_job( j );
	EV_PRINT("%s! type[%d] leave\n", __FUNCTION__, j->iTriggerType);
}
void ftp(struct JOB_STRUCT *j){
	if ( jobStatus.iRamMp4Counter <RAM_MP4_MAX ){
		EV_PRINT("%s! type[%d] enter\n", __FUNCTION__, j->iTriggerType);
		sleep(1);
	}
	next_job(j);
	EV_PRINT("%s! type[%d] leave\n", __FUNCTION__, j->iTriggerType);
	return ;
}


