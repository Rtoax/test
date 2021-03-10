/**
 *  测试F-Stack的UDP发包速率
 *  作者：荣涛 <rongtao@sylincom.com>
 *  时间：2021年3月9日10:55:27
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "common.h"

int sockfd = 0;

void sig_handler(int signum)
{
    printf("Catch ctrl+C(SIGINT) signal.\n");
    close(sockfd);
    exit(1);
}

#ifdef MAXLINE
#undef MAXLINE
#define MAXLINE 2048
#endif

int main(int argc, char *argv[])
{
    signal(SIGINT, sig_handler);
    printf("---\n");
    int n;
    struct sockaddr_in servaddr;
    socklen_t servlen = sizeof(servaddr);
    
    struct timeval tvbrfore, tvafter;
    
    long int npkg = 0, nbyte=0;
    
    char sendline[MAXLINE + 1] = {"abcdefghijklmnopqrstuvwxyz"};
    char recvline[MAXLINE + 1];
    
    if(argc != 3) {
        perror("Usage: udpcli <IPAddress> <port>\n");
        exit(1);
    }
    int port  = atoi(argv[2]);

	int send_Nbytes = MAXLINE;

	if(argc > 3) {
		send_Nbytes = atoi(argv[3])?atoi(argv[3]):MAXLINE;
	}
   
    printf("%s:%d\n", argv[1], port);
 
    sockfd = udpsocket_client(argv[1], port, &servaddr);
    
    npkg = 0, nbyte=0;
    memset(&tvbrfore, 0, sizeof(struct timeval));
    memset(&tvafter, 0, sizeof(struct timeval));

	while(1) {
	    if((n = sendto(sockfd, sendline, send_Nbytes, 0, 
			        (struct sockaddr *)&servaddr, servlen)) <= 0)
		{
			perror("sendto error");
			exit(1);
		}
	}

    close(sockfd);

    return 1;
}
