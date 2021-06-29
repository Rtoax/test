/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:28 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:32 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 13日 星期三 08:54:03 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 08日 星期五 08:10:16 CST. */
/* Copyright (c) Colorado School of Mines, CST.*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:27:53 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:26:59 CST. */
/*
int setrlimit (int resource, const struct rlimit *rlp) [Function]
Store the current and maximum limits for the resource resource in *rlp.
The return value is 0 on success and -1 on failure. The following errno error condition
is possible:
EPERM
• The process tried to raise a current limit beyond the maximum limit.
• The process tried to raise a maximum limit, but is not superuser.
When the sources are compiled with _FILE_OFFSET_BITS == 64 on a 32-bit system
this function is in fact setrlimit64. Thus, the LFS interface transparently replaces
the old interface.
*/
/*
 *	n RLIMIT_CPU—The maximum CPU time, in seconds, used by a program.This is
 *	the amount of time that the program is actually executing on the CPU, which is
 *	not necessarily the same as wall-clock time. If the program exceeds this time
 *	limit, it is terminated with a SIGXCPU signal.
 *	n RLIMIT_DATA—The maximum amount of memory that a program can allocate
 *	for its data. Additional allocation beyond this limit will fail.
 *	n RLIMIT_NPROC—The maximum number of child processes that can be running
 *	for this user. If the process calls fork and too many processes belonging to this
 *	user are running on the system, fork fails.
 *	n RLIMIT_NOFILE—The maximum number of file descriptors that the process may
 *	have open at one time.
 */
#include <sys/resource.h>
#include <stdio.h>
#include <unistd.h>

// struct rlimit{
    // rlim_t  rlim_cur;
    // rlim_t  rlim_max;
// };

int demo_cpu_time_limit()
{
	struct rlimit rl;
	/* Obtain the current limits. */
	getrlimit (RLIMIT_CPU, &rl);
	/* Set a CPU limit of 1 second. */


    printf("rlimit.rlim_cur = %d\n", rl.rlim_cur);
    printf("rlimit.rlim_max = %d\n", rl.rlim_max);
    
	rl.rlim_cur = 1;
	setrlimit (RLIMIT_CPU, &rl);
	/* Do busy work. */
	while (1); //超出 CPU 时限(吐核)
	return 0;
}

int main()
{
#if 1
	demo_cpu_time_limit();
#endif

    struct rlimit rlimit, rlimit2;
    
    rlimit.rlim_cur = 12;
    rlimit.rlim_max = 13;
    
    setrlimit(getpid(), &rlimit);
    
    getrlimit(getpid(), &rlimit2);

    printf("rlim_cur = %d\n", rlimit2.rlim_cur);
    printf("rlim_max = %d\n", rlimit2.rlim_max);

    
	while (1); //超出 CPU 时限(吐核)
	
	return 0;
}
