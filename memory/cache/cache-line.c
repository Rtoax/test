/**
 *  缓存行 对 内存IO的影响
 *  作者：荣涛
 *  时间：2021年4月2日
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syscall.h>

enum op_type {
    TASK_RD,
    TASK_WR,
};

#ifndef __cacheline_size
#define __cacheline_size 64 //默认为 64 字节
#endif

struct test_cache_line {
    unsigned long val1;
    unsigned long __pad[7];
    unsigned long val2;
}__attribute__((aligned(__cacheline_size)));


struct task_arg{
    enum op_type peer_type;
    unsigned long *pdata;
};

#define RWDR_TIMES  100000000

static void read_test(unsigned long *pdata) {
    int i;
    unsigned long tmp = 0;
    for(i=0; i< RWDR_TIMES; i++)
        tmp = *pdata;
}
static void write_test(unsigned long *pdata) {
    int i;
    unsigned long tmp = 0;
    for(i=0; i< RWDR_TIMES; i++)
        *pdata = tmp++;
}


static void* task_routine(void* arg)
{
    static unsigned long time_usec[2] = {0};

    struct task_arg *Arg = (struct task_arg*)arg;

    struct timeval start, end;
	printf("start task tid %ld.\n", syscall(__NR_gettid));
    gettimeofday(&start, NULL);

    switch(Arg->peer_type) {
        case TASK_RD: 
            read_test(Arg->pdata);
            break;
        case TASK_WR: 
            write_test(Arg->pdata);
            break;
        default: 

            break;
    }

    gettimeofday(&end, NULL);

    time_usec[Arg->peer_type] = end.tv_sec*1000000 + end.tv_usec \
                             - (start.tv_sec*1000000 + start.tv_usec);
    
    pthread_exit(&time_usec[Arg->peer_type]);
	return NULL;
}

int main ()
{
    unsigned long *read_usec, *write_usec;
    
	pthread_t tasks[2];
    long cacheline_size = sysconf(_SC_LEVEL3_CACHE_LINESIZE);
    
    printf("cache L3 line size = %d\n", cacheline_size);

    if(__cacheline_size != cacheline_size) {
        printf("You should #define __cacheline_size %ld.\n", cacheline_size);
        return -1;
    }

    struct test_cache_line test_data = {1024UL, 2048UL};

    struct task_arg tastArg[2] = {
        {TASK_RD, &test_data.val1},
        {TASK_WR, &test_data.val2},
        };
    
	pthread_create(&tasks[TASK_RD], NULL, task_routine, &tastArg[0]);
	pthread_create(&tasks[TASK_WR], NULL, task_routine, &tastArg[1]);

	pthread_join(tasks[TASK_RD], (void**)&read_usec);
	pthread_join(tasks[TASK_WR], (void**)&write_usec);

    printf("Total read  = %ld MSec\n", *read_usec);
    printf("Total write = %ld MSec\n", *write_usec);

	return 0;
}


