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
   2019年 03月 26日 星期二 08:37:42 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 26日 星期二 08:37:34 CST. */

/**
 *	把文件描述符锁住了
 */
#include <stdio.h>
#include <fcntl.h>

#define Fcntl fcntl

void my_lock(int fd)
{
	struct flock	lock;

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;				/* write lock entire file */

	Fcntl(fd, F_SETLKW, &lock);
}

void my_unlock(int fd)
{
	struct flock	lock;

	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;				/* unlock entire file */

	Fcntl(fd, F_SETLK, &lock);
}


int main()
{
	my_lock(fileno(stdout));

	printf("123\n");

	my_unlock(fileno(stdout));
	
	return 0;
}
