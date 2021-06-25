#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <signal.h>

#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

#include <fastlog.h>

#include "common.h"





int benchmark_staticstring(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "staticstring", "Static String Benchmark: critical\n");
    FAST_LOG(FASTLOG_WARNING,   "staticstring", "Static String Benchmark: warning\n");
    FAST_LOG(FASTLOG_ERR,       "staticstring", "Static String Benchmark: error\n");
    FAST_LOG(FASTLOG_INFO,      "staticstring", "Static String Benchmark: information\n");
    FAST_LOG(FASTLOG_DEBUG,     "staticstring", "Static String Benchmark: debug\n");

    return 5;
}
int benchmark_singleint(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "singleint", "Single Int Benchmark: critical\n");
    FAST_LOG(FASTLOG_WARNING,   "singleint", "Single Int Benchmark: warning\n");
    FAST_LOG(FASTLOG_ERR,       "singleint", "Single Int Benchmark: error\n");
    FAST_LOG(FASTLOG_INFO,      "singleint", "Single Int Benchmark: information\n");
    FAST_LOG(FASTLOG_DEBUG,     "singleint", "Single Int Benchmark: debug\n");

    return 5;
}

int benchmark_complex(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_WARNING, "TEST1", "[%s] CPU %d(%d)\n", "hello", __fastlog_sched_getcpu(), __fastlog_getcpu());
    FAST_LOG(FASTLOG_ERR, "TEST1", "%f %f %f\n", 3.14, 3.14, 3.14);
    FAST_LOG(FASTLOG_CRIT, "TEST1", "%2.3f %2.3f %2.3lf\n", 3.14, 3.14, 3.14);
    FAST_LOG(FASTLOG_INFO, "TEST1", "%d %d %ld %d %d %d %s\n", 1, 2, 3L, 1, 2, 3, "Hello");
    FAST_LOG(FASTLOG_WARNING, "TEST1", ">>># %d %ld %ld %s %f #<<<\n", 1, 2L, 3L, "Hello", 1024.0);

    FAST_LOG(FASTLOG_CRIT, "TEST2", ">>># I have an integer %ld #<<<\n", total_dequeue);
    FAST_LOG(FASTLOG_ERR, "TEST2", ">>># I have an integer %ld #<<<\n", total_dequeue+10);
    FAST_LOG(FASTLOG_INFO, "TEST2", ">>># I have an integer %ld #<<<\n", total_dequeue+11);
    FAST_LOG(FASTLOG_CRIT, "TEST21", ">>># I have an integer %ld #<<<\n", total_dequeue);
    FAST_LOG(FASTLOG_INFO, "TEST21", ">>># I have an integer %ld #<<<\n", total_dequeue+11);

    FAST_LOG(FASTLOG_INFO, "TEST3", ">>># Hello %s 1#<<<\n", "World");
    FAST_LOG(FASTLOG_ERR, "TEST3", ">>># Hello %s 3#<<<\n", "World");
    FAST_LOG(FASTLOG_ERR, "TEST3", ">>># Hello %*s 3#<<<\n", 10, "World");
    FAST_LOG(FASTLOG_WARNING, "TEST3", ">>># Hello %.*s 3#<<<\n", 4, "World");
    FAST_LOG(FASTLOG_WARNING, "TEST3", ">>># Hello %*.*s 3#<<<\n", 20, 4, "World");
    FAST_LOG(FASTLOG_DEBUG, "TEST3", ">>># Hello %.*ld %.*ld 3#<<<\n", 7, total_dequeue, 7, total_dequeue+1);
    FAST_LOG(FASTLOG_DEBUG, "TEST3", ">>># Hello %*.*ld 3#<<<\n", 20, 7, total_dequeue+10);
    FAST_LOG(FASTLOG_INFO, "TEST3", ">>># Hello %*.*ld 3#<<<\n", 30, 7, total_dequeue+20);
    FAST_LOG(FASTLOG_ERR, "TEST3", ">>># Hello %*s 3#<<<\n", 10, "World");
    FAST_LOG(FASTLOG_WARNING, "TEST3", ">>># Hello %.*s 3#<<<\n", 4, "World");

    return 20;
}

static void *task_routine(void*param) 
{
    struct task_arg * arg = (struct task_arg *)param;
    
    set_thread_cpu_affinity(arg->cpu);    
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    unsigned long total_dequeue = 0;

    while(1) {
        
        total_dequeue += benchmark_singleint(total_dequeue);

        if(total_dequeue % 10000000 == 0) {
            
            gettimeofday(&end, NULL);

            unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
            double nmsg_per_sec = (double)((total_dequeue)*1.0 / usec) * 1000000;
            printf("\nTotal = %ld, %ld/sec\n", total_dequeue, (unsigned long )nmsg_per_sec);
            
            sleep(1);
            total_dequeue = 0;

            gettimeofday(&start, NULL);
        }
    }
    pthread_exit(arg);
}



void test_benchmark()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int itask;
    pthread_t threads[64];
    int nthread = 3;
    char thread_name[32] = {0};

    for(itask=0; itask<nthread; itask++) {
        
        struct task_arg *arg = malloc(sizeof(struct task_arg));
        arg->cpu = itask%ncpu;
        
        pthread_create(&threads[itask], NULL, task_routine, arg);
        sprintf(thread_name, "rtoax:%d", itask);
        pthread_setname_np(threads[itask], thread_name);
        memset(thread_name, 0, sizeof(thread_name));
    }
    
    for(itask=0; itask<nthread; itask++) {
        struct task_arg *arg = NULL;
        pthread_join(threads[itask], (void**)&arg);
        free(arg);
    }
    
    
}

