/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#include <fcntl.h> /* Definition of AT_* constants */
#include <unistd.h>

//read value of a symbolic link relative to a directory file descriptor
int readlinkat(int dirfd, const char *pathname,
              char *buf, size_t bufsiz);
