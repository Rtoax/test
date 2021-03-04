#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
/* According to POSIX.1-2001 */
#include <sys/select.h>

#include "utils.h"
#include "common.h"


/*
    在实时内核下 /proc/interrupts 的下面中断会暴增
                CPU0       CPU1       CPU2       CPU3
     RES:  179392251  155305295  176785837  176676556   Rescheduling interrupts

    操作系统、内核和实时补丁版本例如下：
 # uname -r
    3.10.0-1127.rt56.1093.el7.x86_64
    
 # cat /etc/redhat-release 
    CentOS Linux release 7.4.1708 (Core)
*/

atomic64_t read_total;

void *read_task(void*arg){
    struct task_arg *ARG = (struct task_arg *)arg;
    struct select_context *sctx = ARG->sctx;
    
    int i =0, nmsg = 0, imsg = 0, ret, nfds;
    eventfd_t count = 1;
    
    reset_self_sched(ARG->sched_policy, ARG->sched_priority);
    reset_self_cpuset(ARG->cpu_list);

    log_dequeue("\n");
    sleep(1);
    
    for (;;) {
        sctx->readset = sctx->allset;
        nfds = select(sctx->maxfd+1, &sctx->readset, NULL, NULL, NULL);
        if (nfds == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }
//        log_dequeue("select return\n");
        for (i = 0; i < MAX_EVENTS; ++i) {
            if(!FD_ISSET(sctx->producer[i], &sctx->readset)) {
                continue;
            }
            ret = eventfd_read(sctx->producer[i], &count);
            if(ret < 0) {
                break;
            }
            
            atomic64_add(&read_total, count);
//            sleep(1);
//            log_dequeue("read fd = %d, count = %ld\n", sctx->producer[i], count);
            for(imsg = 0; imsg < count; ++imsg) {
                
            }
        }
    }
    pthread_exit(NULL);
}

void *write_task(void*arg) {
    
    struct task_arg *ARG = (struct task_arg *)arg;
    struct select_context *sctx = ARG->sctx;

    reset_self_sched(ARG->sched_policy, ARG->sched_priority);
    reset_self_cpuset(ARG->cpu_list);
    
    int evt_fd = eventfd_create();
    select_add_fd(sctx, evt_fd);
    
    int i =0;
    int ret;
    eventfd_t count = 1;

//    log_enqueue("\n");
    
    while(1) {
//        sleep(1);
//        log_enqueue("write ret = %d, count = %ld\n", ret, count);
        ret = eventfd_write(evt_fd, count);
        if(ret < 0) {
            continue;
        }
    }
    pthread_exit(NULL);
}

void sig_handler(int signum)
{
	log_dequeue("Total read %ld\n", atomic64_read(&read_total));
	exit(0);
}



int main(int argc, char *argv[])
{
    int i;
    int nwrite = 3;
    
	signal(SIGINT, sig_handler);
    
    pthread_t dequeue, enqueue[10];
    
    struct task_arg dequeue_arg;
    struct task_arg enqueue_arg[10];

    struct select_context sctx;
    char *cpu_lists[10] = {
        "0",    //dequeue
        "1",    //enqueue1
        "2",    //enqueue2
        "3",    //enqueue3
        "4",    //enqueue4
        "5",    //enqueue5
        "6",    
        "7",    
        "8",    
        "9",    
    };
        
    if(argc > 1) {
        for(i=0;i<argc && i<nwrite+1;i++) {
            if (argv[i+1]) {
                printf("%d == %s:%s\n", i, cpu_lists[i], argv[i+1]);
                cpu_lists[i] = strdup(argv[i+1]);
            }
        }
    }
    select_context_init(&sctx);
    
    atomic64_init(&read_total);
    
    dequeue_arg.sctx = &sctx;
    dequeue_arg.sched_policy = SCHED_OTHER;
    dequeue_arg.sched_priority = 0;
    dequeue_arg.cpu_list = strdup(cpu_lists[0]);
    pthread_create(&dequeue, NULL, read_task, &dequeue_arg);
    pthread_setname_np(dequeue, "Dequeue");

    for(i=0;i<nwrite;i++) {
        char threadname[64] = {0};
        enqueue_arg[i].sctx = &sctx;
        enqueue_arg[i].sched_policy = SCHED_OTHER;
        enqueue_arg[i].sched_priority = 0;
        enqueue_arg[i].cpu_list = strdup(cpu_lists[i+1]);
        pthread_create(&enqueue[i], NULL, write_task, &enqueue_arg[i]);
        snprintf(threadname, 64, "Enqueue%d", i+1);
        pthread_setname_np(enqueue[i], threadname);
    }
    pthread_setname_np(pthread_self(), "Main");

    pthread_join(dequeue, NULL);
    for(i=0;i<nwrite;i++) {
        pthread_join(enqueue[i], NULL);
    }

    return EXIT_SUCCESS;
}


