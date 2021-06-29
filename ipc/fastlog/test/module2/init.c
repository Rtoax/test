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
#include <module2.h>

#include "common.h"

void module2_init()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    
    struct task_arg *arg2 = malloc(sizeof(struct task_arg));
    
    arg2->cpu = 1%ncpu;
    
    pthread_create(&module_threads[MODULE_2], NULL, task_module2, arg2);

    
    pthread_setname_np(module_threads[MODULE_2], modules_tasks_name[MODULE_2]);
}





