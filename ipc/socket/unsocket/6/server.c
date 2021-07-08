#include "common.h"

void main()
{
    int listen_fd;
    int com_fd; 
    int ret = 0;
    int i;
    int len;
    struct sockaddr_un clt_addr;
    
    listen_fd = unsocket_server(UNIX_DOMAIN);
    //printf("SSIZE_MAX = %d\n", SSIZE_MAX);
    while(1)
    {
        len = sizeof(clt_addr);
        
        com_fd = accept(listen_fd, (struct sockaddr*)&clt_addr, &len);
#if 0
		int nSendBuf=32*1024;//设置为32K
		setsockopt(com_fd,SOL_SOCKET,SO_RCVBUF,(const char*)&nSendBuf,sizeof(int));
		setsockopt(com_fd,SOL_SOCKET,SO_OOBINLINE,NULL, sizeof(int));
#endif
		int val = 1;
		setsockopt(listen_fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val));
		setsockopt(com_fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val));
        if(com_fd < 0)
        {
            perror("cannot accept client connect request.");
            close(listen_fd);
            unlink(UNIX_DOMAIN);
            break;
        }
        
        memset(buf, 0, MSG_LENGTH);

		while(1)
		{
			len = recv(com_fd, buf, MSG_LENGTH, 0);
			printf("%d: len= %d, recv: %s\n", com_fd, len, buf);
			memset(buf, 0, MSG_LENGTH);
			if(len<=0)
				break;
		}

        buf[0] = 'X';
        
        send(com_fd, buf, MSG_LENGTH, 0);
        
        close(com_fd);
    }
}
