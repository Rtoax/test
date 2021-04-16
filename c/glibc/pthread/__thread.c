//__thread int i;
//extern __thread struct state s;
//static __thread char *p;


#include <pthread.h>
#include <stdio.h>

struct test1 {
     int  id;
};

__thread struct test1 t;
__thread int tid = 0;
//thread_local  int tl_tid = 0;

/**
当标记有 __thread 存储类说明符的变量，都将被视为多线程应用程序中每个线程的本地变量。
在运行时，为访问他的每个线程创建变量的副本，并在线程结束时销毁。

*/

void* test_task_fn(void* unused)
{
	printf("test_task_fn.\n");

    tid ++;
    t.id ++;

    pthread_exit(NULL);
	return NULL;
}

/* The main program. */
int main ()
{
	pthread_t t1, t2;

    sleep(1);
	pthread_create(&t1, NULL, test_task_fn, NULL);
    sleep(1);
	pthread_create(&t2, NULL, test_task_fn, NULL);


	pthread_join(t1, NULL);

	pthread_join(t2, NULL);

    printf("tid = %d\n", tid);
    printf("t.id = %d\n", t.id);

	return 0;
}


