/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#include <fcntl.h> /* Definition of AT_* constants */
#include <sys/stat.h>

//mkdirat - create a directory relative to a directory file descriptor
int mkdirat(int dirfd, const char *pathname, mode_t mode);



SYSCALL_DEFINE3(mkdirat, int, dfd, const char __user *, pathname, umode_t, mode)
{
	return do_mkdirat(dfd, pathname, mode);
}

