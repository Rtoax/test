#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <time.h>

#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>

#include <fastlog.h>

#include "common.h"


void *task_routine(void*param) 
{
    struct task_arg * arg = (struct task_arg *)param;
    
    set_thread_cpu_affinity(arg->cpu);
    
//    FAST_LOG(FASTLOG_WARNING, "TEST", "[%s] CPU %d(%ld)\n", "hello", __fastlog_sched_getcpu(), __fastlog_getcpu());
//    FAST_LOG(FASTLOG_WARNING, "TEST", "%f %lf %llf\n", 3.14, 3.14, 3.14L);
//    FAST_LOG(FASTLOG_WARNING, "TEST", "%2.3f %2.3lf %2.3llf\n", 3.14, 3.14, 3.14L);
//    FAST_LOG(FASTLOG_WARNING, "TEST", "%d %ld %lld %d %ld %lld\n", 1, 2, 3L, 1, 2, 3L);
    FAST_LOG(FASTLOG_WARNING, "TEST", "Hello world\n");
    FAST_LOG(FASTLOG_WARNING, "TEST", "%f %lf %llf %p %s\n", 3.14, 3.14, 3.14L, arg, "Hello");
    pthread_exit(arg);
}

int main()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int icpu, itask;

    pthread_t threads[64];
    int nthread = 1;

    fastlog_init();

    for(itask=0; itask<nthread; itask++) {
        
        struct task_arg *arg = malloc(sizeof(struct task_arg));
        arg->cpu = itask%ncpu;
        
        pthread_create(&threads[itask], NULL, task_routine, arg);
    }
    
    for(itask=0; itask<nthread; itask++) {
        struct task_arg *arg = NULL;
        pthread_join(threads[itask], &arg);
        free(arg);
    }
    
	return 0;
}

