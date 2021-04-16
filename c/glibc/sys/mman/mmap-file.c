/*************************************************************************
	> File Name: mmap-file.c
	> Author:  荣涛
	> Mail:     
	> Created Time: 2021年4月16日17:19:23
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/mman.h>
#include<unistd.h>
#include<sys/types.h>
#include<wait.h>

int value = 100;
int fd;

void sig_handler(int signo) {
    close(fd);//映射区创建完之后，文件描述符就没影响了
    unlink("test.txt.out");
    exit(0);
}

int main()
{
    int i = 1;
    int *p = NULL;
    pid_t pid;

    signal(SIGINT, sig_handler);
    
    //截断打开/创建文件
    fd = open("test.txt.out",O_RDWR|O_CREAT|O_TRUNC,0644);
    if(fd == -1)
    {
        perror("open\n");
        exit(1);
    }
    if((ftruncate(fd,8)) == -1)
    {
        perror("ftruncate\n");
        exit(1);
    }

    //创建映射区
    p = mmap(NULL,4,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
    if(p == MAP_FAILED)
    {
        perror("mmap\n");
        exit(1);
    }

    while(1) {
        sleep(3);
        *p = i++;
        printf("write %d\n", *p);
    }
    
    sig_handler(0);
    
    return 0;
}

