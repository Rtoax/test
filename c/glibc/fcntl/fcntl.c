/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:11 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:24 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 13日 星期三 08:53:54 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 12日 星期二 15:07:31 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 08日 星期五 08:10:08 CST. */
/* Copyright (c) Colorado School of Mines, CST.*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:27:47 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:26:54 CST. */
/*	int fcntl (int filedes, int command, . . .) [Function]
 *	The fcntl function performs the operation specifed by command on the fle de
 *	scriptor fledes. Some commands require additional arguments to be supplied. These
 *	additional arguments and the return value and error conditions are given in the de
 *	tailed descriptions of the individual commands.
 *	Brieﬂy, here is a list of what the various commands are.
 *	F_DUPFD Duplicate the fle descriptor (return another fle descriptor pointing to the
 *	same open fle). See Section 13.12 [Duplicating Descriptors], page 330.
 *	F_GETFD Get ﬂags associated with the fle descriptor. See Section 13.13 [File De
 *	scriptor Flags], page 331.
 *	F_SETFD Set ﬂags associated with the fle descriptor. See Section 13.13 [File De
 *	scriptor Flags], page 331.
 *	F_GETFL Get ﬂags associated with the open fle. See Section 13.14 [File Status
 *	Flags], page 333.
 *	F_SETFL Set ﬂags associated with the open fle. See Section 13.14 [File Status
 *	Flags], page 333.
 *	F_GETLK Get a fle lock. See Section 13.15 [File Locks], page 338.
 *	F_SETLK Set or clear a fle lock. See Section 13.15 [File Locks], page 338.
 *
 *	F_SETLKW Like F_SETLK, but wait for completion. See Section 13.15 [File Locks],
 *	page 338.
 *	F_GETOWN Get process or process group ID to receive SIGIO signals. See
 *	Section 13.16 [Interrupt-Driven Input], page 341.
 *	F_SETOWN Set process or process group ID to receive SIGIO signals. See Section 13.16
 *	[Interrupt-Driven Input], page 341.
 *	This function is a cancellation point in multi-threaded programs. This is a problem
 *	if the thread allocates some resources (like memory, fle descriptors, semaphores or
 *	whatever) at the time fcntl is called. If the thread gets canceled these resources stay
 *	allocated until the program ends. To avoid this calls to fcntl should be protected
 *	using cancellation handlers.
 *
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

int fcntl(int filedes, int command, ...);

#define ERR_EXIT(msg) \
	do{ \
		perror(msg);\
		exit(-1);\
	}while(0)

        
/* Set the O_NONBLOCK ﬂag of desc if value is nonzero,
or clear the ﬂag if value is 0.
Return 0 on success, or -1 on error with errno set. */
int set_nonblock_flag (int desc, int value)
{
    int oldflags = fcntl (desc, F_GETFL, 0);
    
    /* If reading the ﬂags failed, return error indication now. */
    if (oldflags == -1)
        return -1;
    
    /* Set just the ﬂag we want to set. */
    if (value != 0)
        oldflags |= O_NONBLOCK;
    else
        oldflags &= ~O_NONBLOCK;
    
    /* Store modifed ﬂag word in the descriptor. */
    return fcntl (desc, F_SETFL, oldflags);
}

int main()
{
	int fd = fileno(stdout);
	//int fd = fcntl(fileno(stdout), F_DUPFD);	//复制描述符
	char s[10];
	memset(s, 0, 10);
	sprintf(s, "Hello R\n");
	write(fd, &s, 10);
	//printf("s:%s\n", s);


	/*	lock and unlock */
	int fd1 = open("test.dat", O_CREAT|O_RDWR|O_TRUNC, 0644);
	if(fd1 < 0)
	{
		ERR_EXIT("openfile error");
	}

	struct flock f;

	memset(&f, 0, sizeof(struct flock));

	f.l_type	= F_WRLCK;
	f.l_whence	= SEEK_SET;
	f.l_start	= 0;
	f.l_len		= 0;

	write(fd1, "RongTao\nasdf\n", 14);

	if(fcntl(fd, F_SETLK, &f) < 0)
	{
		ERR_EXIT("lock error");
	}
	//getchar();

	f.l_type	= F_UNLCK;

	if(fcntl(fd, F_SETLK, &f) < 0)
	{
		ERR_EXIT("unlock error");
	}


	/* F_GETFD, F_SETFD */
	int stdoutno = fileno(stdout);
	int flagout = fcntl(stdoutno, F_GETFD, 0);
    int newno = fcntl (stdoutno, F_SETFD, flagout);

    printf("stdoutno=%d, flagout=%d, newno=%d\n", stdoutno, flagout, newno);

	return 0;
}
