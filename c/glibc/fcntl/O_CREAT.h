/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:11 CST. */


// O_CREAT
// create the file if it does not already exist
//
// Posix.1 guarantees that if the open function is called with the 0-CREAT (create the file if
// it does not already exist) and 0-EXCL flags (exclusive open), the function returns an
// error if the file already exists. Furthermore, the check for the existence of the file and
// the creation of the file (if it does not already exist) must be atomic with regard to other
// processes. We can therefore use the file created with this technique as a lock. We are
// guaranteed that only one process at a time can create the file (i.e., obtain the lock), and
// to release the lock, we just unlink the file.
//
//
