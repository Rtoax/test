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
#include <module1.h>

#include "common.h"

void module1_init()
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);

    struct task_arg *arg1 = malloc(sizeof(struct task_arg));
    
    arg1->cpu = 0%ncpu;

    M1_INFO("Module1 init. cpu %d\n", arg1->cpu);
    
    pthread_create(&module_threads[MODULE_1], NULL, task_module1, arg1);

    pthread_setname_np(module_threads[MODULE_1], modules_tasks_name[MODULE_1]);

    
    M1_INFO("Module1 init done. cpu %d\n", arg1->cpu);
}

