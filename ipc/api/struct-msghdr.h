/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 06月 14日 星期五 09:06:04 CST. */
/* 
 Copyright (C) Rong Tao @Beijing

 Permission is granted to copy, distribute and/or modify this document
 under the terms of the GNU Free Documentation License, Version 1.3
 or any later version published by the Free Software Foundation;
 with no Invariant Sections, no Front-Cover Texts, and no Back-Cover
 Texts. A copy of the license is included in the section entitled ‘‘GNU
 Free Documentation License’’.
   2019年 03月 14日 星期四 19:24:58 CST. 
*/
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 08日 星期五 08:10:39 CST. */
/* Copyright (C) Rong Tao @Sylincom Beijing, 2019年 03月 07日 星期四 20:28:11 CST. */
/**
 *	msghdr结构体
 */
struct msghdr
{
	void			*msg_name;		//protocol address
	socklen_t		msg_namelen;	//size of protocol address
	struct iovec 	*msg_iov;		//scatter/ gather array 分散/收集数组
	int				msg_iovlen;		//elements in msg_iov
	void			*msg_control;	//ancillary data 辅助的
	socklen_t		msg_controllen;	//length of ancillary data
	int				msg_flags;		//falgs returned by recvmsg
};
