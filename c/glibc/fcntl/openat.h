/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

//NAME
//openat - open a file relative to a directory file descriptor

//SYNOPSIS
#include <fcntl.h>

int openat(int dirfd, const char *pathname, int flags);
int openat(int dirfd, const char *pathname, int flags, mode_t mode);

//Feature Test Macro Requirements for glibc (see feature_test_macros(7)):

//openat():
//   Since glibc 2.10:
//       _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L
//   Before glibc 2.10:
//       _ATFILE_SOURCE
