#include "common.h"

void main()
{
    int listen_fd;
    int com_fd; 
    int ret = 0;
    int i = 0;
    int len;
    struct sockaddr_un clt_addr;

    struct MsgHdr *msghdr = (struct MsgHdr *)buf;
    char *data = (char*)msghdr->data;
    
    listen_fd = unsocket_server(UNIX_DOMAIN);
    //printf("SSIZE_MAX = %d\n", SSIZE_MAX);
    
    while(1)
    {
        i++;
        
        len = sizeof(clt_addr);
        
        com_fd = accept(listen_fd, (struct sockaddr*)&clt_addr, &len);

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

        msghdr->src = 1;
        msghdr->dst = 2;
        
        memset(data, 0, MSG_LENGTH-sizeof(struct MsgHdr));

		while(1)
		{
			len = recv(com_fd, msghdr, MSG_LENGTH, 0);
			printf("(%d->%d:%d) len %3d, recv: %s\n", msghdr->src, msghdr->dst, msghdr->id, len, msghdr->data);
			memset(msghdr, 0, MSG_LENGTH);
			if(len<=0)
				break;
		}

        data[0] = 'X';
        
        send(com_fd, msghdr, len, 0);
        
        close(com_fd);
    }
}
