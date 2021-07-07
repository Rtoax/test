/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:07 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:21 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 12日 星期二 15:07:28 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 08日 星期五 08:10:06 CST. */
/* Copyright (c) Colorado School of Mines, CST.*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:27:45 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:26:52 CST. */
/*
 *	6.35 Inquiring on Alignment of Types or Variables
 *	The keyword __alignof__ allows you to inquire about how an object is aligned, or the
 *	minimum alignment usually required by a type. Its syntax is just like sizeof.
 *	For example, if the target machine requires a double value to be aligned on an 8-byte
 *	boundary, then __alignof__ (double) is 8. This is true on many RISC machines. On
 *	more traditional machine designs, __alignof__ (double) is 4 or even 2.
 *	Some machines never actually require alignment; they allow reference to any data type
 *	even at an odd address. For these machines, __alignof__ reports the smallest alignment
 *	that GCC will give the data type, usually as mandated by the target ABI.
 *	If the operand of __alignof__ is an lvalue rather than a type, its value is the required
 *	alignment for its type, taking into account any minimum alignment specifed with GCC’s
 *	__attribute__ extension (see Section 6.36 [Variable Attributes], page 329). For example,
 *	after this declaration:
 *	struct foo { int x; char y; } foo1;
 *	the value of __alignof__ (foo1.y) is 1, even though its actual alignment is probably 2 or
 *	4, the same as __alignof__ (int).
 *	It is an error to ask for the alignment of an incomplete type.
 */

#include <stdio.h>

int main()
{
	printf("%d\n", __alignof__(double));
	printf("%d\n", __alignof__(int));
	printf("%d\n", __alignof__(unsigned long long));
}
