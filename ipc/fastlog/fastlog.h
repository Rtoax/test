/**
 *  FastLog 低时延 LOG日志
 *
 *
 *
 */

#ifndef __fAStLOG_H
#define __fAStLOG_H 1

#define __USE_GNU
#define _GNU_SOURCE
#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <syscall.h>
#include <stdbool.h>


static inline int __sched_getcpu() { 
    return sched_getcpu(); 
}

static inline unsigned __getcpu() {
    unsigned cpu, node;
    int ret = syscall(__NR_getcpu, &cpu, &node);
    return cpu;
}









#endif /*<__fAStLOG_H>*/
