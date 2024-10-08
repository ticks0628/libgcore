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

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file threadpool.h
 * @brief Threadpool Header File
 */
 
 /**
 * Increase this constants at your own risk
 * Large values might slow down your system
 */
#define MAX_THREADS 50
#define MAX_QUEUE 100
typedef enum {
	ALIVE		=0,
	IMMEDIATE_SHUTDOWN= 1,
	GRACEFUL_SHUTDOWN = IMMEDIATE_SHUTDOWN<<1
} threadpool_shutdown_t;

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

//typedef int*(CBFUNC_MOTION) ( int window );

typedef	void *(THREAD_FUNC_PTR)(void *arg);
typedef struct {
	THREAD_FUNC_PTR	*func_ptr;
	//void* (func_ptr)(void *arg);
    void *argument;
    int	function_type;
} threadpool_task_t;

/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_type  Array means is this a thread(always holding) or a normal function(release thread when function return).
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */
struct threadpool_t {
  pthread_mutex_t	lock;
  pthread_cond_t	notify;
  threadpool_task_t	*queue;
  pthread_t		*threads;
  int			*thread_type;
  int			*thread_busy;
  THREAD_FUNC_PTR	**thread_func;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
};

typedef struct threadpool_t threadpool_t;

typedef enum {
	THREADPOOL_INVALID	= -1,
	THREADPOOL_LOCK_FAILURE = -2,
	THREADPOOL_QUEUE_FULL	= -3,
	THREADPOOL_SHUTDOWN	= -4,
	THREADPOOL_THREAD_FAILURE = -5
} threadpool_error_t;

/**
 * @function threadpool_create
 * @brief Creates a threadpool_t object.
 * @param thread_count Number of worker threads.
 * @param queue_size   Size of the queue.
 * @param flags        Unused parameter.
 * @return a newly created thread pool or NULL
 */
int threadpool_create(threadpool_t *pool, int thread_count, int queue_size);

/**
 * @function threadpool_add
 * @brief add a new task in the queue of a thread pool
 * @param pool     Thread pool to which add the task.
 * @param function Pointer to the function that will perform the task.
 * @param argument Argument to be passed to the function.
 * @param flags    Unused parameter.
 * @return 0 if all goes well, negative values in case of error (@see
 * threadpool_error_t for codes).
 */
int threadpool_add(threadpool_t *pool, void* (function)(void *), void *arg, int flags);
int threadpool_del(threadpool_t *pool, void* (function)(void *));

/**
 * @function threadpool_destroy
 * @brief Stops and destroys a thread pool.
 * @param pool  Thread pool to destroy.
 * @param flags Flags for shutdown
 *
 * Known values for flags are 0 (default) and threadpool_graceful in
 * which case the thread pool doesn't accept any new tasks but
 * processes all pending tasks before shutdown.
 */
int threadpool_destroy(threadpool_t *pool, int flags);
int threadpool_status(threadpool_t *pool);

#ifdef __cplusplus
}
#endif

#endif /* _THREADPOOL_H_ */
