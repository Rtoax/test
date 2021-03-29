#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

void* test_task_fn(void* unused)
{
	printf("test_task_fn.\n");

    static int status = 12121;
 
	sleep(1);

    pthread_exit(&status);
	return NULL;
}
/* The main program. */
int main ()
{
    int *pstatus;
	pthread_t thread_id;
    
	pthread_create(&thread_id, NULL, test_task_fn, NULL);

	//pthread_join(thread_id, (void**)&pstatus);

	int ret = pthread_kill(thread_id, 0);
	printf("ret = %d, EINVAL = %d, ESRCH = %d\n", ret, EINVAL, ESRCH);

	//ret = pthread_kill(0, 0);
	//printf("ret = %d\n", ret);
	pthread_join(thread_id, (void**)&pstatus);

	printf("pstatus = %d\n", *pstatus);
	ret = pthread_kill(thread_id, 0);
	printf("ret = %d\n", ret);
	return 0;
}

