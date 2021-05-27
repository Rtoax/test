/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

// SPDX-License-Identifier: GPL-2.0
#define _GNU_SOURCE
#include <fcntl.h>

//ssize_t splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);

//[rongtao@localhost splice]$ echo asdfadsf | ./default_file_splice_read.out 
//asdfadsf

int main(int argc, char **argv)
{
        splice(0, 0, 1, 0, 1<<30, 0);
	return 0;
}
