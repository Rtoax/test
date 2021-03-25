/**
 *  UNIX套接字示例代码
 *
 *  作者：荣涛
 *  时间：2020年11月9日
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <sys/mman.h>

#ifndef UNIX_DOMAIN
#define UNIX_DOMAIN "./rtoax_unsocket_pcap"
#endif

#define MSG_LENGTH          1024

#define MAX_CLIENT_NUM      10

#define debug() printf("%s:%d\n", __func__, __LINE__)

extern char                 buf[MSG_LENGTH];

int unsocket_server(const char *PATH);
int unsocket_client(const char *PATH);

void memshow(const char *prefix, void *ptr, ssize_t size);


#endif /*<_COMMON_H>*/
