/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 24日 星期一 08:52:25 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:05:19 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:28 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 13日 星期三 09:02:19 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 13日 星期三 08:53:59 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 08日 星期五 08:10:12 CST. */
/* Copyright (c) Colorado School of Mines, CST.*/
/* All rights reserved.                       */

/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:27:50 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:26:57 CST. */
/**
 *	互斥锁：上锁和解锁
 */

#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *mptr);
int pthread_mutex_trylock(pthread_mutex_t *mptr);
int pthread_mutex_unlock(pthread_mutex_t *mptr);

/**
 *	如果尝试给一个已由另外一个线程锁住的互斥锁上锁，那么pthread_mutex_lock将
 *	阻塞到该互斥锁解锁为止。pthread_mutex_trylock是对应的非阻塞函数，如果该
 *	互斥锁已锁住，它就返回一个EBUSY错误。
 */

//返回：
//		成功：0
//		错误：正数：Exxx值

/**
 *	如果试图上锁已被另外某个线程锁住的一个互斥锁，本线程将被阻塞，
 *	知道该互斥锁被解锁位置。
 */

/**
 *	example02  有点问题，先放着吧
 */
#include <stdio.h>
#include <stdlib.h>

#define NLOOP 5000
int counter;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

void *doit(void *);

int main(int argc, char **argv)
{
	pthread_t tidA, tidB;

	pthread_create(&tidA, NULL, &doit, NULL);
	pthread_create(&tidB, NULL, &doit, NULL);

	/*等待两个线程终止*/
	pthread_join(tidA, NULL);
	pthread_join(tidB, NULL);

	exit(0);
}

void *doit(void *vptr)
{
	int i, val;

	for(i=0; i<NLOOP; i++)
	{
		pthread_mutex_lock(&counter_mutex);

		val = counter;

		printf("%d: %d\n", pthread_self(), val+1);

		pthread_mutex_unlock(&counter_mutex);
	}
	return NULL;
}

