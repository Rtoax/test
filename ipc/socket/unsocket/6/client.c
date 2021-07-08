#include "common.h"

void main()
{
    int connect_fd;
    int ret = 0, len;

    connect_fd  = unsocket_client(UNIX_DOMAIN);
    int val = 1;
	setsockopt(connect_fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val));
    printf("fd: %d\n", connect_fd);
	while(1)
	{
		scanf("%s", buf);
		//buf[strlen(buf)] = '\r';
		//buf[strlen(buf)] = '\n';
		int i;
		for(i=0;i<1;i++)
		{
			ret = send(connect_fd, buf, strlen(buf), 0); 
			//ret = write(connect_fd, buf, MSG_LENGTH);//知道接收长度
			//ret = send(connect_fd, buf, strlen(buf), 0);
			printf("send bytes = %d: %s\n", ret, buf);
			//sleep(1);
			//flush(connect_fd);

		}
	}

    len = recv(connect_fd, buf, MSG_LENGTH, 0);
    
    printf("%d:%s\n", len, buf);
    
    close(connect_fd);

}
