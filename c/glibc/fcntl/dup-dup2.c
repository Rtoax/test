/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:11 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:24 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 13日 星期三 08:53:54 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 12日 星期二 15:07:31 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 08日 星期五 08:10:08 CST. */
/* Copyright (c) Colorado School of Mines, CST.*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:27:47 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:26:54 CST. */
/*	13.12 Duplicating Descriptors
 *	You can duplicate a fle descriptor, or allocate another fle descriptor that refers to the same
 *	open fle as the original. Duplicate descriptors share one fle position and one set of fle
 *	status ﬂags (see Section 13.14 [File Status Flags], page 333), but each has its own set of fle
 *	descriptor ﬂags (see Section 13.13 [File Descriptor Flags], page 331).
 *	The major use of duplicating a fle descriptor is to implement redirection of input or
 *	output: that is, to change the fle or pipe that a particular fle descriptor corresponds to.
 *	You can perform this operation using the fcntl function with the F_DUPFD command,
 *	but there are also convenient functions dup and dup2 for duplicating descriptors.
 *	The fcntl function and ﬂags are declared in ‘fcntl.h’, while prototypes for dup and
 *	dup2 are in the header fle ‘unistd.h’.
 *	int dup (int old) [Function]
 *	This function copies descriptor old to the frst available descriptor number (the frst
 *	number not currently open). It is equivalent to fcntl (old, F_DUPFD, 0).
 *	int dup2 (int old, int new) [Function]
 *	This function copies the descriptor old to descriptor number new.
 *	If old is an invalid descriptor, then dup2 does nothing; it does not close new. Other
 *	wise, the new duplicate of old replaces any previous meaning of descriptor new, as if
 *	new were closed frst.
 *	If old and new are diﬀerent numbers, and old is a valid descriptor number, then dup2
 *	is equivalent to:
 *	close (new);
 *	fcntl (old, F_DUPFD, new)
 *	However, dup2 does this atomically; there is no instant in the middle of calling dup2
 *	at which new is closed and not yet a duplicate of old.
 *	int F_DUPFD [Macro]
 *	This macro is used as the command argument to fcntl, to copy the fle descriptor
 *	given as the frst argument.
 *	The form of the call in this case is:
 *	fcntl (old, F_DUPFD, next-filedes)
 *	The next-fledes argument is of type int and specifes that the fle descriptor returned
 *	should be the next available one greater than or equal to this value.
 *	The return value from fcntl with this command is normally the value of the new
 *	fle descriptor. A return value of −1 indicates an error. The following errno error
 *	conditions are defned for this command:
 *	EBADF The old argument is invalid.
 *	EINVAL The next-fledes argument is invalid.
 *	EMFILE There are no more fle descriptors available—your program is already
 *	using the maximum. In BSD and GNU, the maximum is controlled by a
 *	resource limit that can be changed; see Section 22.2 [Limiting Resource
 *	Usage], page 575, for more information about the RLIMIT_NOFILE limit.
 *	ENFILE is not a possible error code for dup2 because dup2 does not create a new
 *	opening of a fle; duplicate descriptors do not count toward the limit which ENFILE
 *	indicates. EMFILE is possible because it refers to the limit on distinct descriptor
 *	numbers in use in one process.
 *	Here is an example showing how to use dup2 to do redirection. Typically, redirection
 *	of the standard streams (like stdin) is done by a shell or shell-like program before calling
 *	one of the exec functions (see Section 26.5 [Executing a File], page 691) to execute a new
 *	program in a child process. When the new program is executed, it creates and initializes
 *	the standard streams to point to the corresponding fle descriptors, before its main function
 *	is invoked.
 *	So, to redirect standard input to a fle, the shell could do something like:
 *	pid = fork ();
 *	if (pid == 0)
 *	{
 *	char *filename;
 *	char *program;
 *	int file;
 *	...
 *	file = TEMP_FAILURE_RETRY (open (filename, O_RDONLY));
 *	dup2 (file, STDIN_FILENO);
 *	TEMP_FAILURE_RETRY (close (file));
 *	execv (program, NULL);
 *	}
 *	There is also a more detailed example showing how to implement redirection in the
 *	context of a pipeline of processes in Section 27.6.3 [Launching Jobs], page 704.
 *
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define TEMP_FAILURE_RETRY(cmd) \
	do{\
	__label__ _again; \
	_again: \
	if(cmd != 0) \
		goto _again;\
	}while(0)


int main()
{
	pid_t pid = fork ();

	if (pid == 0)
	{
		char *filename;
		char *program;
		int file;
		
		file = TEMP_FAILURE_RETRY (open (filename, O_RDONLY));
		dup2 (file, fileno(stdin));
		TEMP_FAILURE_RETRY (close (file));
		execv (program, NULL);
	}

	return 0;
}

