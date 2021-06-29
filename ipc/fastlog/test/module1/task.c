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

#define HANDLE(i)   handle_msg##i
#define HANDLE_DEF(i)                   \
static inline void HANDLE(i)()          \
{                                       \
    M1_WARN("msg%d.\n", i);             \
}
#define CASE_HANDLE(i) \
    case i: HANDLE(i)(); break;


HANDLE_DEF(0);
HANDLE_DEF(1);
HANDLE_DEF(2);
HANDLE_DEF(3);
HANDLE_DEF(4);
HANDLE_DEF(5);



static inline void handle_msgs()
{
    static unsigned long msg_cnt = 0;
    msg_cnt++;

    M1_INFO("Recv %ld msg.\n", msg_cnt);

    switch(msg_cnt%10) {
        CASE_HANDLE(0); 
        CASE_HANDLE(1); 
        CASE_HANDLE(2); 
        CASE_HANDLE(3); 
        CASE_HANDLE(4); 
        CASE_HANDLE(5); 
        default:
            M1_ERR("msg %ld.\n", msg_cnt%10);
            break;
    }
    
}

void *task_module1(void*param) 
{    
    struct task_arg * arg = (struct task_arg *)param;
    
    M1_WARN("Module1 task startup. cpu %d\n", arg->cpu);
    
    set_thread_cpu_affinity(arg->cpu);    
   
    unsigned long total_dequeue = 0;

    while(1) {

        total_dequeue += 3;
    
        M1_CRIT("module 1 %ld\n", total_dequeue);
        
        handle_msgs();
        
        usleep(30000);
    }
    pthread_exit(arg);
}

