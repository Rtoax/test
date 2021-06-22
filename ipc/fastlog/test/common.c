#define __USE_GNU
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

int set_thread_cpu_affinity(int i)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
 
    CPU_SET(i,&mask);
 
//    printf("setaffinity thread %u(tid %d) to CPU%d\n", pthread_self(), gettid(), i);
    if(-1 == pthread_setaffinity_np(pthread_self() ,sizeof(mask),&mask)) {
        printf("pthread_setaffinity_np error.\n");
        return -1;
    }
    return 0;
}