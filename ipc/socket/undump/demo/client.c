/**
 *  UNIX套接字示例代码：客户端
 *  
 *  作者：荣涛
 *  时间：2020年11月9日
 */
#include "common.h"

void main()
{
    int connect_fd;
    int ret = 0, len;

    connect_fd  = unsocket_client(UNIX_DOMAIN);
    int val = 1;
	setsockopt(connect_fd, SOL_TCP, TCP_NODELAY, &val, sizeof(val));
    printf("fd: %d\n", connect_fd);
	while(1) {
	    //printf("INPUT>>");
		//scanf("%s", buf);
		sprintf(buf, "abcdefghijklmnopqrstuvwxyz.123456789");
        
		ret = write(connect_fd, buf, strlen(buf)); 
		printf("TX bytes %d: %s\n", ret, buf);
        memshow("-- ", buf, ret);
		sleep(1);
	}

    len = read(connect_fd, buf, MSG_LENGTH);
    
    printf("%d:%s\n", len, buf);
    
    close(connect_fd);

}
