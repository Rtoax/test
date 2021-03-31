#include <pthread.h>
#include <stdio.h>
#include <time.h>

int linux_taskDelete( unsigned long tid)
{
#if 1
	//printf("tid = %lu\n", tid);
    int ret = 0;
    if(tid == 0) {
        printf("[ERROR] Try to cancel [tid=0] pthread task.\n");
        goto task_del_error;
    }
    //线程存在,可 cancel
    if((ret=pthread_kill(tid,0)) == 0) {
        return pthread_cancel( tid );
    } else {
        printf("[ERROR] Try to cancel NO EXIST pthread task.\n");
    }
task_del_error:    
    return ret;
#else
    return pthread_cancel( tid );
#endif  
}

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

	linux_taskDelete(thread_id);

	//pthread_timedjoin_np(thread_id, (void**)&pstatus, &abstime);
	pthread_join(thread_id, (void**)&pstatus);
	
	
	sleep(1);
    linux_taskDelete(thread_id+100000);
	linux_taskDelete(0);
	linux_taskDelete(thread_id);

//    printf("pstatus = %d\n", *pstatus);
	return 0;
}




