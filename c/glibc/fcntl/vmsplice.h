/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <fcntl.h>
#include <sys/uio.h>

//vmsplice - splice user pages into a pipe
ssize_t vmsplice(int fd, const struct iovec *iov,
                unsigned long nr_segs, unsigned int flags);

//The  vmsplice()  system  call  maps nr_segs ranges of user memory described by iov into a pipe.  The file
//descriptor fd must refer to a pipe.
//
//The pointer iov points to an array of iovec structures as defined in <sys/uio.h>:
//
//   struct iovec {
//       void  *iov_base;            /* Starting address */
//       size_t iov_len;             /* Number of bytes */
//   };
//
//The flags argument is a bit mask that is composed by ORing together zero or more of the following values:
//
//SPLICE_F_MOVE      Unused for vmsplice(); see splice(2).
//
//SPLICE_F_NONBLOCK  Do not block on I/O; see splice(2) for further details.
//
//SPLICE_F_MORE      Currently has no effect for vmsplice(), but may be  implemented  in  the  future;  see
//                  splice(2).
//
//SPLICE_F_GIFT      The  user  pages are a gift to the kernel.  The application may not modify this memory
//                  ever, or page cache and on-disk data may differ.  Gifting pages to  the  kernel  means
//                  that  a  subsequent  splice(2)  SPLICE_F_MOVE can successfully move the pages; if this
//                  flag is not specified, then a subsequent splice(2) SPLICE_F_MOVE must copy the  pages.
//                  Data must also be properly page aligned, both in memory and length.

