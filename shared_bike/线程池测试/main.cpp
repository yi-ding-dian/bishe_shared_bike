#include <stdio.h>
#include <unistd.h>
#include "threadpool/thread.h"
#include "threadpool/thread_pool.h"

void thread_test_1(void* arg)
{
	while (1)
	{
		printf("thread_test_1\n\n");
		sleep(1);
	}
	
}

void thread_test_2(void* arg)
{
	while (1)
	{
		printf("thread_test_2\n\n");
		sleep(1);
	}

}

int main()
{

	thread_pool_t *threadPool_1 = thread_pool_init();

	thread_task_t* tast_1 = thread_task_alloc(0);
	thread_task_t* tast_2 = thread_task_alloc(0);


	tast_1->handler = thread_test_1;
	tast_2->handler = thread_test_2;

	thread_task_post(threadPool_1, tast_1);
	thread_task_post(threadPool_1, tast_2);

	sleep(100);

	return 0;
}