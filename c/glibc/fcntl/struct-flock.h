/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:11 CST. */


#ifdef __x86_64__
/* Not necessary, we always have 64-bit offsets.  */
# define F_GETLK64	5	/* Get record locking info.  */
# define F_SETLK64	6	/* Set record locking info (non-blocking).  */
# define F_SETLKW64	7	/* Set record locking info (blocking).	*/
#endif


struct flock
{
	short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
	short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
#ifndef __USE_FILE_OFFSET64
	__off_t l_start;	/* Offset where the lock begins.  */
	__off_t l_len;	/* Size of the locked area; zero means until EOF.  */
#else
	__off64_t l_start;	/* Offset where the lock begins.  */
	__off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
#endif
	__pid_t l_pid;	/* Process holding the lock.  */
};

#ifdef __USE_LARGEFILE64
struct flock64
{
	short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
	short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
	__off64_t l_start;	/* Offset where the lock begins.  */
	__off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
	__pid_t l_pid;	/* Process holding the lock.  */
};
#endif



