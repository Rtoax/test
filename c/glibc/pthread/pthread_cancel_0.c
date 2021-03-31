#include <pthread.h>
#include <stdio.h>
#include <time.h>

       

void* test_task_fn(void* unused)
{
	printf("test_task_fn.\n");

    sleep(1);

    static int status = 12121;
    
    pthread_exit(&status);
	return NULL;
}
/* The main program. */
int main ()
{
    int *pstatus;
	pthread_t thread_id;
    struct timespec abstime = {1,1};;

    time_t abst = time(&abst); //man 2 time
    abstime.tv_sec = abst+1;
    
	pthread_create(&thread_id, NULL, test_task_fn, NULL);

	pthread_timedjoin_np(thread_id, (void**)&pstatus, &abstime);
//	pthread_join(thread_id, (void**)&pstatus);

    pthread_cancel(thread_id);
	pthread_cancel(0);

//    printf("pstatus = %d\n", *pstatus);
	return 0;
}




