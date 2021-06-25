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

static void *task_module1(void*param) 
{
    struct task_arg * arg = (struct task_arg *)param;
    
    set_thread_cpu_affinity(arg->cpu);    
   
    unsigned long total_dequeue = 0;

    while(1) {

        total_dequeue += 3;
    
        FAST_LOG(FASTLOG_CRIT, MODULE(MODULE_1), "module 1 %ld\n", total_dequeue);
    
        
        sleep(1);
    }
    pthread_exit(arg);
}

static void *task_module2(void*param) 
{
    struct task_arg * arg = (struct task_arg *)param;
    
    set_thread_cpu_affinity(arg->cpu);    
   
    unsigned long total_dequeue = 0;

    while(1) {

        total_dequeue += 3;
    
        FAST_LOG(FASTLOG_CRIT, MODULE(MODULE_2), "module 2 %ld\n", total_dequeue);
    
        
        sleep(1);
    }
    pthread_exit(arg);
}

static void *task_module3(void*param) 
{
    struct task_arg * arg = (struct task_arg *)param;
    
    set_thread_cpu_affinity(arg->cpu);    
   
    unsigned long total_dequeue = 0;

    while(1) {

        total_dequeue += 3;
    
        FAST_LOG(FASTLOG_CRIT, MODULE(MODULE_3), "module 1 %ld\n", total_dequeue);
    
        
        sleep(1);
    }
    pthread_exit(arg);
}

void modules_init()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    
    pthread_t threads[64];

    struct task_arg *arg1 = malloc(sizeof(struct task_arg));
    struct task_arg *arg2 = malloc(sizeof(struct task_arg));
    struct task_arg *arg3 = malloc(sizeof(struct task_arg));
    
    arg1->cpu = 0%ncpu;
    arg2->cpu = 1%ncpu;
    arg3->cpu = 2%ncpu;
    
    pthread_create(&threads[MODULE_1], NULL, task_module1, arg1);
    pthread_create(&threads[MODULE_2], NULL, task_module2, arg2);
    pthread_create(&threads[MODULE_3], NULL, task_module3, arg3);

    
    pthread_setname_np(threads[MODULE_1], modules_name[MODULE_1]);
    pthread_setname_np(threads[MODULE_2], modules_name[MODULE_2]);
    pthread_setname_np(threads[MODULE_3], modules_name[MODULE_3]);

    struct task_arg *arg = NULL;
    pthread_join(threads[MODULE_1], (void**)&arg);
    free(arg);
    pthread_join(threads[MODULE_2], (void**)&arg);
    free(arg);
    pthread_join(threads[MODULE_3], (void**)&arg);
    free(arg);
}



