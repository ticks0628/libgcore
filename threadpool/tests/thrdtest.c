#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"

int tasks = 0, done = 0;
pthread_mutex_t lock;

void* dummy_task(void *arg) {
	usleep(3000000);
	pthread_mutex_lock(&lock);
	done++;
	pthread_mutex_unlock(&lock);
	return NULL;
}

int main(int argc, char **argv){
	threadpool_t	pool;
	int		ret;

	pthread_mutex_init(&lock, NULL);

	assert((threadpool_create(&pool, 10, 20)) == 0);
	printf("Pool started with %d threads and queue size of %d\n", 10, 20);

	ret=threadpool_add(&pool, dummy_task, NULL, 1);
	sleep(1);
	ret=threadpool_del(&pool, dummy_task);
	

	while( (ret=threadpool_add(&pool, dummy_task, NULL, 1)) == 0) {
		pthread_mutex_lock(&lock);
		tasks++;
		pthread_mutex_unlock(&lock);
	}

	printf("Added %d tasks, ret=%d\n", tasks, ret);

	while((tasks / 2) > done) {
		usleep(10000);
	}

	printf("destroy all thread!\n");
	//assert(threadpool_destroy( &pool, GRACEFUL_SHUTDOWN) == 0);
	assert(threadpool_destroy( &pool, IMMEDIATE_SHUTDOWN) == 0);
	printf("Did %d tasks\n", done);

    return 0;
}
