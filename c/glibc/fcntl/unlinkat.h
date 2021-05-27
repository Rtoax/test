/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

//unlinkat - remove a directory entry relative to a directory file descriptor

#include <fcntl.h> /* Definition of AT_* constants */
#include <unistd.h>

int unlinkat(int dirfd, const char *pathname, int flags);

//Feature Test Macro Requirements for glibc (see feature_test_macros(7)):

//unlinkat():
//   Since glibc 2.10:
//       _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
//   Before glibc 2.10:
//       _ATFILE_SOURCE

