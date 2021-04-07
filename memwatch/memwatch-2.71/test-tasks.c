#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <malloc.h>

#include "memwatch.h"


void alloc_test(int flag) {
    char *p;
    p = vos_malloc(210);
    vos_free(p);
    p = vos_malloc(20);
    p = vos_malloc(200);    /* causes unfreed error */
    p[-1] = 0;          /* causes underflow error */
    vos_free(p);
}

void *task_fn(void *arg) {
    while(1) {
        sleep(1);
        alloc_test(1);
    }
}

void signal_handler(int signum) {
    switch(signum) {
        case SIGALRM:
            printf("alarm.\n");
            alarm(1);
            break;

        case SIGINT:
            printf("Catch ctrl-C signal.\n");
            mwStatistics( MW_STAT_LINE );
            CHECK();
            mwAbort();
            exit(1);
            break;

        default:
            break;
    }
}

int main() {
    pthread_t tasks[10];
    int i;

    signal(SIGALRM, signal_handler);
    signal(SIGINT, signal_handler);

    alarm(1);
    
    mwInit();
    /* Collect stats on a line number basis */
//    mwStatistics( MW_STAT_LINE );
    
//    CHECK();
//    mwAbort();

    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    mwSetAriFunc( mwAriHandler );

    for(i=0; i<10; i++) {
        pthread_create(&tasks[i], NULL, task_fn, NULL);
    }
    
    for(i=0; i<10; i++) {
        pthread_join(tasks[i], NULL);
    }
}
