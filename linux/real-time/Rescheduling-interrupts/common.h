#ifndef __cOMmON_H
#define __cOMmON_H 1

#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX_EVENTS 10




struct epoll_context {
    int epollfd;

    struct epoll_event events[MAX_EVENTS];
    
};

struct select_context {
    int listenfd;
    int maxfd;
    fd_set readset, allset;
    pthread_rwlock_t rwlock;
    int producer[MAX_EVENTS];
};

struct task_arg {
    union {
        struct epoll_context *ectx;
        struct select_context *sctx;
    };
    int sched_policy;
    int sched_priority;
    char *cpu_list;
};


int epoll_context_init(struct epoll_context *ctx);
void epoll_add_fd(struct epoll_context *ctx, int fd);


int eventfd_create();


void reset_self_sched(int sched_policy, int sched_priority);
void reset_self_cpuset(char *cpu_list);


#endif
