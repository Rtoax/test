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


user_rwlock_t rwlock_sum1 = USER_RWLOCK_INITIALIZER;
user_rwlock_t rwlock_sum2 = USER_RWLOCK_INITIALIZER;

unsigned long sum1 =  0;
unsigned long sum2 =  0;

void * task_read_routine(void *arg){
    int j, i = (int)arg;
    int tmp;
    
    char name[16] = {0};
    char s_num[16] = {0};
    snprintf(name, 16, "task%d", i);
    snprintf(s_num, 16, "%d", i%sysconf(_SC_NPROCESSORS_ONLN));
	pthread_setname_np(pthread_self(), name);
    reset_self_cpuset(s_num);


    for(j=0;j<NR_LOOP;j++) {
//        user_rwlock_write_lock(&rwlock_sum2);
//        sum2 ++;
//        user_rwlock_write_unlock(&rwlock_sum2);
        
        user_rwlock_read_lock(&rwlock_sum1);
        printf("sum1 = %ld, j = %d\n", sum1, j);
        user_rwlock_read_unlock(&rwlock_sum1);
        usleep(10);
    }
    

    pthread_exit(NULL);
}


void * task_write_routine(void *arg){
    int j, i = (int)arg;
    
    char name[16] = {0};
    char s_num[16] = {0};
    snprintf(name, 16, "task%d", i);
    snprintf(s_num, 16, "%d", i%sysconf(_SC_NPROCESSORS_ONLN));
	pthread_setname_np(pthread_self(), name);
    reset_self_cpuset(s_num);
    
    for(j=0;j<NR_LOOP;j++) {
        user_rwlock_write_lock(&rwlock_sum1);
        sum1 ++;
        user_rwlock_write_unlock(&rwlock_sum1);
        
//        user_rwlock_read_lock(&rwlock_sum2);
//        printf("sum2 = %ld\n", sum2);
//        user_rwlock_read_unlock(&rwlock_sum2);
    }
    

    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {

    int i;
    
    pthread_t readtasks[NR_TASKS];
    pthread_t writetasks[NR_TASKS];

    

    for(i =0 ; i<NR_TASKS; i++) {
        pthread_create(&readtasks[i], NULL, task_read_routine, (void*)(i+12));
        pthread_create(&writetasks[i], NULL, task_write_routine, (void*)(i+10));
    }
	pthread_setname_np(pthread_self(), "main");
    
    for(i =0 ; i<NR_TASKS; i++) {
	    pthread_join(readtasks[i], NULL);
	    pthread_join(writetasks[i], NULL);
    }

    printf("sum1 = %ld\n", sum1);
    printf("sum2 = %ld\n", sum2);

    return 0;
}

