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

struct benchmark {
    const char *intro;
    int (*test_func)(unsigned long total_dequeue);
};



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
    int i = total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "singleint", "Single Int Benchmark: critical %d\n", i);
    FAST_LOG(FASTLOG_WARNING,   "singleint", "Single Int Benchmark: warning  %d\n", i);
    FAST_LOG(FASTLOG_ERR,       "singleint", "Single Int Benchmark: error  %d\n", i);
    FAST_LOG(FASTLOG_INFO,      "singleint", "Single Int Benchmark: information  %d\n", i);
    FAST_LOG(FASTLOG_DEBUG,     "singleint", "Single Int Benchmark: debug  %d\n", i);

    return 5;
}

int benchmark_twoint(unsigned long total_dequeue)
{
    int i = total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "singleint", "Single Int Benchmark: critical %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_WARNING,   "singleint", "Single Int Benchmark: warning  %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_ERR,       "singleint", "Single Int Benchmark: error  %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_INFO,      "singleint", "Single Int Benchmark: information  %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_DEBUG,     "singleint", "Single Int Benchmark: debug  %d %d\n", i, i+1);

    return 5;
}


int benchmark_singlelong(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "singleint", "Single Int Benchmark: critical %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_WARNING,   "singleint", "Single Int Benchmark: warning  %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_ERR,       "singleint", "Single Int Benchmark: error  %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_INFO,      "singleint", "Single Int Benchmark: information  %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_DEBUG,     "singleint", "Single Int Benchmark: debug  %ld\n", total_dequeue);

    return 5;
}
int benchmark_twolong(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "singleint", "Single Int Benchmark: critical %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_WARNING,   "singleint", "Single Int Benchmark: warning  %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_ERR,       "singleint", "Single Int Benchmark: error  %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_INFO,      "singleint", "Single Int Benchmark: information  %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_DEBUG,     "singleint", "Single Int Benchmark: debug  %ld %ld\n", total_dequeue, total_dequeue);

    return 5;
}


int benchmark_complex(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_WARNING, "TEST1", "[%s] CPU %d(%d)\n", "hello", __fastlog_sched_getcpu(), __fastlog_getcpu());
    FAST_LOG(FASTLOG_ERR, "TEST1", "%f %f %f\n", 3.14, 3.14, 3.14);
    FAST_LOG(FASTLOG_CRIT, "TEST1", "%2.3f %2.3f %2.3lf\n", 3.14, 3.14, 3.14);
    FAST_LOG(FASTLOG_INFO, "TEST1", "%d %d %ld %d %d %d %s\n", 1, 2, 3L, 1, 2, 3, "Hello");
    FAST_LOG(FASTLOG_WARNING, "TEST1", ">>># %d %ld %ld %s %f #<<<\n", 1, 2L, 3L, "Hello", 1024.0);
    
    FAST_LOG(FASTLOG_CRIT, "TEST3", ">>># Hello %.*ld %.*ld 3#<<<\n", 7, total_dequeue, 7, total_dequeue+1);
    FAST_LOG(FASTLOG_DEBUG, "TEST3", ">>># Hello %*.*ld 3#<<<\n", 20, 7, total_dequeue+10);
    FAST_LOG(FASTLOG_INFO, "TEST3", ">>># Hello %*.*ld 3#<<<\n", 30, 7, total_dequeue+20);
    FAST_LOG(FASTLOG_ERR, "TEST3", ">>># Hello %*s 3#<<<\n", 10, "World");
    FAST_LOG(FASTLOG_WARNING, "TEST3", ">>># Hello %.*s 3#<<<\n", 4, "World");

    return 10;
}

#define N_Benchmark  (sizeof(benchmarks)/sizeof(benchmarks[0]))    

static struct benchmark benchmarks[] = {
    {"StaticString",   benchmark_staticstring},
    {"SingleInt",      benchmark_singleint},
    {"TwoInt",          benchmark_twoint},
    {"SingleLong",      benchmark_singlelong},
    {"TwoLong",      benchmark_twolong},
    {"Complex",         benchmark_complex},
};


int __thread idx_benchmark = 0;
int __thread (*test_func)(unsigned long total_dequeue);

unsigned long __thread total_dequeue = 0;

static void *task_routine(void*param) 
{
    struct task_arg * arg = (struct task_arg *)param;
    
    set_thread_cpu_affinity(arg->cpu);    


    test_func = benchmarks[idx_benchmark].test_func;
    
    
    struct timeval start, end;
    gettimeofday(&start, NULL);

    while(1) {
        
        total_dequeue += test_func(total_dequeue);
        if(total_dequeue % 1000000 == 0) {
            

            static unsigned int statistics_count = 0;

            gettimeofday(&end, NULL);

            unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
            double nmsg_per_sec = (double)((total_dequeue)*1.0 / usec) * 1000000;
            
            printf("%-5d %-20s %-15ld\n", statistics_count,
                            benchmarks[idx_benchmark].intro,
                            (unsigned long )nmsg_per_sec);

            statistics_count ++;
            
            total_dequeue = 0;
            idx_benchmark++;
            
            if(idx_benchmark >= N_Benchmark) break;
            
            test_func = benchmarks[idx_benchmark].test_func;

            gettimeofday(&start, NULL);
        }
    }

    
    fprintf(stderr, "exit-----------\n");
    pthread_exit(arg);
}



void test_benchmark()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int itask;
    pthread_t threads[64];
    int nthread = 1;
    char thread_name[32] = {0};

    printf("%-5s %-20s %-15s\n", "NUM", "Benchmark", "(Logs/s)");

    printf("-----------------------------------------\n");

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

