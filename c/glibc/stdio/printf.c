/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:24 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:30 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 13日 星期三 10:11:36 CST. */
#include <stdio.h>
#include <string.h>

#define STR	"hello world"
int main()
{
	int i;
	for (i=0; i<=strlen(STR);i++)
		printf("%3d/%d  [%.*s]\n", i, strlen(STR), i, STR);

	printf("KB/s \n");

	//printf("%*.*d\n", 3, 10);
	printf("%.*s\n", 4, "hello");
	printf("%*s\n", 40, "hell0");
	printf("%*.*s\n", 40, 4, "hell0");

	printf("%.*d\n", 4, 10000);
	printf("%*d\n", 40, 10000);
	printf("%*.*d\n", 40, 4, 10000);
	return 0;
}
