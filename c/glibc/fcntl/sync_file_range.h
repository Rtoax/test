/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <fcntl.h>

//sync_file_range - sync a file segment with disk
int sync_file_range(int fd, off64_t offset, off64_t nbytes,
                   unsigned int flags);

//The flags bit-mask argument can include any of the following values:
//
//  SYNC_FILE_RANGE_WAIT_BEFORE
//         Wait  upon  write-out  of all pages in the specified range that have already been submitted to the
//         device driver for write-out before performing any write.
//
//  SYNC_FILE_RANGE_WRITE
//         Initiate write-out of all dirty pages in the specified range which  are  not  presently  submitted
//         write-out.  Note that even this may block if you attempt to write more than request queue size.
//
//  SYNC_FILE_RANGE_WAIT_AFTER
//         Wait upon write-out of all pages in the range after performing any write.

