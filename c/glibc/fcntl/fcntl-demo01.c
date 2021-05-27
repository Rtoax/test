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
/**
 *	fcntl
 *
 *		之前介绍的connect，recv，send都是阻塞性函数，若资源没有准备
 *	好，则调用该函数的进程将进入睡眠状态，这样就无法处理I/O多路复用
 *	的情况。
 *		该出两种解决I/O多路复用的解决方法，这两个函数分别为：fcntl和
 *	select。
 *
 *	函数fcntl针对socket编程提供了如下特性：
 *		非阻塞I/O：可将cmd设置为F_SETFL，将lock设置为O_NONBLOCK；
 *		信号驱动I/O：可将cmd这是为F_SETFL，将lock设置为O_ASYNC。
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

#define SERVPORT 3333
#define BACKLOG 10
#define MAX_CONNECTED_NO 10
#define MAXDATASIZE 100

int main()
{
	struct sockaddr_in server_sockaddr, client_sockaddr;
	int sin_size, recvbytes, flags;
	int sockfd, client_fd;
	char buf[MAXDATASIZE];

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		exit(1);
	}

	printf("socket success! sockfd = %d\n", sockfd);
	server_sockaddr.sin_family = AF_INET;	
	server_sockaddr.sin_port = htons(SERVPORT);
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero), 8);

	if(bind(sockfd, 
		(struct sockaddr *)&server_sockaddr, 
		sizeof(struct sockaddr)) == -1)
	{
		perror("bind error");exit(1);
	}

	printf("bind success\n");
	if(listen(sockfd, BACKLOG) == -1)
	{
		perror("listen error");exit(1);
	}
	printf("listenning...");

	/*调用fcntl函数，这只非阻塞参数*/
	if((flags = fcntl(sockfd, F_SETFL, 0)) < 0)
	{
		perror("fcntl F_FETFL error");
		exit(1);
	}

	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETEL, flags) < 0)
	{
		perror("fcntl");
	}

	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		if((client_fd = accept(sockfd,
							(struct sockaddr*)&client_sockaddr,
							&sin_size)) == -1)
		{
			perror("accept error");exit(1);
		}
	
		if((recvbytes = recv(client_fd, buf, MAXFATASIZE, 0)) == -1)
		{
			perror("recv");exit(1);
		}

		if(read(client_fd, buf, MAXDATASIZE) < 0)
		{
			perror("read error");exit(1);
		}
		printf("received a connection: %s\n", buf);
	}
	close(client_fd);
	exit(0);
}
























