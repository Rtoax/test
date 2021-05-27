/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>

//fallocate - manipulate(操作) file space
// 系统调用: 让应用程序预分配一些可能连续的空间。提高之后的写性能
int fallocate(int fd, int mode, off_t offset, off_t len);



int ksys_fallocate(int fd, int mode, loff_t offset, loff_t len)
{
	struct fd f = fdget(fd);
	int error = -EBADF;

	if (f.file) {
		error = vfs_fallocate(f.file, mode, offset, len);
		fdput(f);
	}
	return error;
}

SYSCALL_DEFINE4(fallocate, int, fd, int, mode, loff_t, offset, loff_t, len)
{
	return ksys_fallocate(fd, mode, offset, len);
}

