/*
 * Copyright (c) 2016, Mathias Brossard <mathias@brossard.org>.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/syscall.h>

#include <pthread.h>
#include "threadpool.h"

/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);

int threadpool_create(threadpool_t *pool, int thread_count, int queue_size){
	int i;

	if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
		return -1;
	}

	/* Initialize */
	pool->thread_count = 0;
	pool->queue_size = queue_size;
	pool->head = pool->tail = pool->count = 0;
	pool->shutdown = pool->started = 0;
	/* Allocate thread and task queue */
	pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
	pool->thread_type = (int*)calloc(thread_count, sizeof(int));
	pool->thread_busy = (int*)calloc(thread_count, sizeof(int));
	pool->thread_func = (void*)calloc(thread_count, sizeof(pool->thread_func));
	pool->queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_size);

	/* Initialize mutex and conditional variable first */
	if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
	(pthread_cond_init(&(pool->notify), NULL) != 0) ||
	(pool->threads == NULL) || (pool->queue == NULL)){
		fprintf(stderr, "%s)mutex, notify initial fail!\n", __func__);
		goto err;
	}

	/* Start worker threads */
	for(i = 0; i < thread_count; i++) {
		if(pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0) {
			threadpool_destroy(pool, 0);
			fprintf(stderr, "%s)pthread create[%d] fail!\n", __func__, i);
			return -1;
		}
		//printf("thread[%d]=[%x]\n", i, (int)pool->threads[i]);
		pool->thread_count++;
		pool->started++;
	}
	return 0;

err:
	threadpool_free(pool);
	return -1;
}
int threadpool_del(threadpool_t *pool, THREAD_FUNC_PTR *func ){
	int	i;

	for(i=0; i< pool->thread_count; i++){
		if( pool->thread_func[i] == func ) break;
	}
	if( i >= pool->thread_count){
		fprintf( stderr, "%s) can't found in threadpool!\n", __func__);
		return -1;
	}
	pthread_cancel( pool->threads[i] );

	if(pthread_join(pool->threads[i], NULL) != 0) {
		fprintf( stderr, "%s) pthread join fail!\n", __func__ );
		return THREADPOOL_THREAD_FAILURE;
	}
	//printf("%s) delete thread[%d] %p:%p\n", __func__, i, (void*)pool->thread_func[i], func );
	printf("%s) delete thread[%d] \n", __func__, i );
	pool->thread_busy[i] = 0;
	pool->thread_type[i] = 0;
	pool->thread_func[i] = NULL;
	if(pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0) {
		fprintf(stderr, "%s) pthread create[%d] fail!\n", __func__, i);
		return THREADPOOL_THREAD_FAILURE;
	}
	return 0;
}
int threadpool_add(threadpool_t *pool, void* (function)(void *), void *argument, int function_type){
	int err = 0;

	if(pool == NULL || function == NULL) {
		return THREADPOOL_INVALID;
	}

	if(pthread_mutex_lock(&(pool->lock)) != 0) {
		return THREADPOOL_LOCK_FAILURE;
	}

	do{
	        /* Are we full ? */
		if(pool->count == pool->queue_size) {
			err = THREADPOOL_QUEUE_FULL;
			break;
		}

		/* Are we shutting down ? */
		if(pool->shutdown) {
			err = THREADPOOL_SHUTDOWN;
			break;
		}

		pool->queue[pool->tail].func_ptr= function;
		pool->queue[pool->tail].argument = argument;
		pool->queue[pool->tail].function_type= function_type;
		pool->tail = (pool->tail + 1) % pool->queue_size;
		pool->count += 1;

		/* pthread_cond_broadcast */
		if(pthread_cond_signal(&(pool->notify)) != 0) {
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
	} while(0);

	if(pthread_mutex_unlock(&pool->lock) != 0) {
		err = THREADPOOL_LOCK_FAILURE;
	}
	threadpool_status( pool);
	return err;
}
int threadpool_status(threadpool_t *pool){
	int i, busy=0;

	for(i=0; i<pool->thread_count; i++){
		if( pool->thread_busy[i] ) busy++;
	}
	printf("%s)thread runs [%d/%d]\t", __func__, busy, pool->thread_count);
	printf("thread queue [%d/%d]\n", pool->count, pool->queue_size);
	return 0;
}
int threadpool_destroy(threadpool_t *pool, int shutdown_mode){
    int i, err = 0;

	if(pthread_mutex_lock(&(pool->lock)) != 0) {
		return THREADPOOL_LOCK_FAILURE;
	}

	do {
		/* Already shutting down */
		if(pool->shutdown) {
			err = THREADPOOL_SHUTDOWN;
			break;
		}

		pool->shutdown = shutdown_mode;

		/* Wake up all worker threads */
		if((pthread_cond_broadcast(&(pool->notify)) != 0) || (pthread_mutex_unlock(&(pool->lock)) != 0)) {
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
		if(pool->shutdown & IMMEDIATE_SHUTDOWN){
			for(i = 0; i < pool->thread_count; i++) {
				if( pool->thread_type[i] !=0 ){
					pthread_cancel( pool->threads[i] );
					printf("shoot thread %d\n", i );
				}
			}
		}
		/* Join all worker thread */
		for(i = 0; i < pool->thread_count; i++) {
			printf("pthread_join %d/%d\n", i, pool->thread_count);
			if(pthread_join(pool->threads[i], NULL) != 0) {
				err = THREADPOOL_THREAD_FAILURE;
			}
		}
	} while(0);

	/* Only if everything went well do we deallocate the pool */
	if(!err) {
		threadpool_free(pool);
	}
	return err;
}

int threadpool_free(threadpool_t *pool){
	if( pool->started > 0) {
		return -1;
	}

	/* Did we manage to allocate ? */
	if(pool->threads) {
		free(pool->threads);
		free(pool->queue);
		free(pool->thread_type);
		free(pool->thread_busy);
		free(pool->thread_func);
 
		/* Because we allocate pool->threads after initializing the
		mutex and condition variable, we're sure they're
		initialized. Let's lock the mutex just in case. */
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_unlock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->notify));
	}
	return 0;
}
static void *threadpool_thread(void *threadpool){
	threadpool_t *pool	= (threadpool_t *)threadpool;
	threadpool_task_t	task;
	int			i, whoami=0;

	pthread_t threads = pthread_self();
	for(i = 0; i < pool->thread_count; i++){
		if( pool->threads[i] == threads)	break;
	}
	if( i >= pool->thread_count ){
		fprintf( stderr, "error, can't found correct thread ID\n");
		exit(1);
	}
	whoami = i;
	for(;;) {
		/* Lock must be taken to wait on conditional variable */
		pthread_mutex_lock(&(pool->lock));

		/* Wait on condition variable, check for spurious wakeups.
		When returning from pthread_cond_wait(), we own the lock. */
		while((pool->count == 0) && (!pool->shutdown)) {
			pthread_cond_wait(&(pool->notify), &(pool->lock));
		}

		if((pool->shutdown == IMMEDIATE_SHUTDOWN) ||
			((pool->shutdown == GRACEFUL_SHUTDOWN) && (pool->count == 0))) {
			printf("thread leave\n");
			break;
		}

		/* Grab our task */
		task.func_ptr = pool->queue[pool->head].func_ptr;
		task.argument = pool->queue[pool->head].argument;
		pool->thread_type[whoami] = pool->queue[pool->head].function_type;
		
		pool->head = (pool->head + 1) % pool->queue_size;
		pool->count -= 1;

		/* Unlock */
		pthread_mutex_unlock(&(pool->lock));
		/* Get to work */
		pool->thread_busy[whoami] = 1;
		pool->thread_func[whoami] = task.func_ptr;
		task.func_ptr(task.argument);
		pool->thread_busy[whoami] = 0;
		pool->thread_type[whoami] = 0;
		pool->thread_func[whoami] = NULL;
	}

	pool->started--;

	pthread_mutex_unlock(&(pool->lock));
	pthread_exit(NULL);
	return(NULL);
}
