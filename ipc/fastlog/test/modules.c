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
#include <module2.h>
#include <module3.h>

#include "common.h"

void modules_init()
{
    module1_init();
    module2_init();
    module3_init();
}



void modules_loop()
{
    int i;
    for(i=0;i<MODULE_NUM;i++) {
        if(module_threads[i] != 0) {
            struct task_arg *arg = NULL;
            pthread_join(module_threads[i], (void**)&arg);
            free(arg);
        }
    }
}

