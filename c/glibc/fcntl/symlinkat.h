/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#include <fcntl.h> /* Definition of AT_* constants */
#include <unistd.h>

//symlinkat - create a symbolic link relative to a directory file descriptor
int symlinkat(const char *oldpath, int newdirfd, const char *newpath);


SYSCALL_DEFINE3(symlinkat, const char __user *, oldname,
		int, newdfd, const char __user *, newname)
{
	return do_symlinkat(oldname, newdfd, newname);
}

