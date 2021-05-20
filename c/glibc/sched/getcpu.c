/* Copyright (c) RToax, 2021年 05月 20日 星期四 17:10:54 CST.*/
/* All rights reserved.                       */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <time.h>
//#include <linux/getcpu.h>

#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>

static long getcpu(unsigned *cpu, unsigned *node) {
    int ret;
    ret = syscall(__NR_getcpu, cpu, node);
    return ret;
}

int main()
{
    long int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int icpu, inode;
    
    cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
    
    CPU_SET(ncpu>1?ncpu-1:1, &cpuset);
    
    int ret = sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    
    printf("ret = %d\n", ret);
    
    int j;
    for(j=0;j<CPU_SETSIZE; j++)
    {
        if(CPU_ISSET(j, &cpuset))
        {
            printf("CPU_SETSIZE = %d, j = %d, cpuset = %d\n", CPU_SETSIZE, j, cpuset);
            CPU_CLR(j, &cpuset);
            printf("CPU_SETSIZE = %d, j = %d, cpuset = %d\n", CPU_SETSIZE, j, cpuset);
        }
    }
    
    icpu = inode = -1;
    ret = getcpu(&icpu, &inode);
    printf("getcpu icpu = %d, inode = %d\n", icpu, inode);
    
    ret = sched_getaffinity(getpid(), sizeof(cpuset), &cpuset);

    for(j=0;j<CPU_SETSIZE; j++)
    {
        if(CPU_ISSET(j, &cpuset))
        {
            printf("CPU_SETSIZE = %d, j = %d, cpuset = %d\n", CPU_SETSIZE, j, cpuset);
        }
    }
    
	return 0;
}
