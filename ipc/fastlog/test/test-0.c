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



void signal_handler(int signum)
{
    switch(signum) {
    case SIGINT:
        printf("Catch ctrl-C.\n");
        fastlog_exit();
        break;
    }
    exit(1);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    fastlog_init(FASTLOG_ERR, 1, 10*1024*1024/* 10MB */);

    fastlog_setlevel(FASTLOG_DEBUG);
    
    
    FAST_LOG(FASTLOG_INFO, "MAIN", "start to run...\n");

#if 0
    printf("####### Benchmark #######\n");
    test_benchmark();
    sleep(5);
    printf("####### Benchmark #######\n");
    test_benchmark();
#elif 1
    if(argc == 2) {
        
        if(strcmp(argv[1], "modules") == 0) {
            
            printf("####### Modules #######\n");
            modules_init();

            goto normal_exit;
            
        } else if(strcmp(argv[1], "benchmark") == 0) {
        
            printf("####### Benchmark #######\n");
            test_benchmark();

            goto normal_exit;
        } else {
            goto error_exit;
        }
    } else {
        goto error_exit;
    }

error_exit:
    printf("%s [modules|benchmark].\n", argv[0]);
    
normal_exit:
#endif
    fastlog_exit();
	return 0;
}

