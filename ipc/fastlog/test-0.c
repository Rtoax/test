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
    
    printf("__sched_getcpu = %d, __getcpu = %d\n", __sched_getcpu(), __getcpu());

    pthread_exit(arg);
}

int main()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int icpu, itask;

    pthread_t threads[64];
    int nthread = 4;

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

