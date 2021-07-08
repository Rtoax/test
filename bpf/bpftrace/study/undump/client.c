#include "common.h"

void main()
{
    int connect_fd;
    int ret = 0, len;

    connect_fd  = unsocket_client(UNIX_DOMAIN);
    
    int i;
    int val = 1;
	setsockopt(connect_fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val));

    struct MsgHdr *msghdr = (struct MsgHdr *)buf;
    char *data = (char*)msghdr->data;

	while(1)
	{
		scanf("%s", data);

        msghdr->src = 2;
        msghdr->dst = 1;
        
	    msghdr->id = i++;
        
		ret = send(connect_fd, msghdr, strlen(data)+sizeof(struct MsgHdr), 0); 

		printf("send bytes = %d: %s\n", ret, data);
	}

    len = recv(connect_fd, data, MSG_LENGTH, 0);
    
    printf("%d:%s\n", len, data);
    
    close(connect_fd);

}
