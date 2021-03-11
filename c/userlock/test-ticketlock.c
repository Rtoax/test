#include <stdio.h>
#include <pthread.h>

#include <userlock.h>

#include "utils.h"

#ifndef NR_TASKS
#define NR_TASKS    2
#endif

#ifndef NR_LOOP
#define NR_LOOP 100000
#endif

user_ticketlock_t ticketlock = USER_TICKETLOCK_INITIALIZER;

unsigned long sum =  0;

void * task_routine(void *arg){
    int j, i = (int)arg;
    
    char name[16] = {0};
    char s_num[16] = {0};
    snprintf(name, 16, "task%d", i);
    snprintf(s_num, 16, "%d", i%sysconf(_SC_NPROCESSORS_ONLN));
	pthread_setname_np(pthread_self(), name);
    reset_self_cpuset(s_num);


    for(j=0;j<NR_LOOP;j++) {
        user_ticketlock_lock(&ticketlock);
        sum ++;
        user_ticketlock_unlock(&ticketlock);
    }
    

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    int i;

    pthread_t addtasks[NR_TASKS];

    

    for(i =0 ; i<NR_TASKS; i++) {
        pthread_create(&addtasks[i], NULL, task_routine, (void*)(i+10));
    }
	pthread_setname_np(pthread_self(), "main");
    
    for(i =0 ; i<NR_TASKS; i++) {
	    pthread_join(addtasks[i], NULL);
    }

    printf("sum = %ld\n", sum);

    return 0;
}

