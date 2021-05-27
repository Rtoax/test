/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#include<fcntl.h>

    /* Flags for SPLICE and VMSPLICE.  */
# define SPLICE_F_MOVE		1	/* Move pages instead of copying.  */
# define SPLICE_F_NONBLOCK	2	/* Don't block on the pipe splicing
                           (but we may still block on the fd
                           we splice from/to).  */
# define SPLICE_F_MORE		4	/* Expect more data.  */
# define SPLICE_F_GIFT		8	/* Pages passed in are a gift.  */


//duplicating pipe content
ssize_t tee(int fd_in, int fd_out, size_t len,unsigned int flags);

//tee()  duplicates  up  to len bytes of data from the pipe referred to by the file descriptor fd_in to the
//pipe referred to by the file descriptor fd_out.  It does not consume the data  that  is  duplicated  from
//fd_in; therefore, that data can be copied by a subsequent splice(2).
//
//flags is a series of modifier flags, which share the name space with splice(2) and vmsplice(2):
//
//SPLICE_F_MOVE      Currently has no effect for tee(); see splice(2).
//
//SPLICE_F_NONBLOCK  Do not block on I/O; see splice(2) for further details.
//
//SPLICE_F_MORE      Currently  has  no  effect  for  tee(),  but  may  be  implemented  in the future; see
//                  splice(2).
//
//SPLICE_F_GIFT      Unused for tee(); see vmsplice(2).

