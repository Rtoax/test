/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>

#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>

#define HOLE_FILE   "hole.file.out"
#define HOLE_LSEEK  "hole.lseek.out"
#define HOLE_PREALLOC  "hole.preallocate.out"

//[root@localhost fcntl]# od -c hole.file.out 
//0000000   A   A   A   A   A   A   A   A   A   A   A   A   A   A   A   A
//*
//0010000  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
//*
//0040000   A   A   A   A   A   A   A   A   A   A   A   A   A   A   A   A
//*


int main(int argc, char *argv[])
{
    
    char *buffer = malloc(102400);

    int i;
    for(i=0;i<102400;i++){
        buffer[i]='A';
    }

    int fd;

    fd = open(HOLE_FILE, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if(fd == -1) {
        printf("error open %s\n", HOLE_FILE);
        exit(1);
    }

    write(fd, buffer, 40960);

    //空洞区域 du -sh 不计入， ls计入
    //数据中， 4K-16K之间为空洞，40K-12K=28K
    if(fallocate(fd, FALLOC_FL_PUNCH_HOLE|FALLOC_FL_KEEP_SIZE, 4096, 4096*3)<0) {
        perror("fallocate");
    }
    if(lseek(fd, 4096, SEEK_SET)==-1){
        perror("lseek");
    }
    char c;
    if(read(fd, &c, 1)==-1){
        perror("read");
    }
    read(fd, &c, 1);
    printf("%c\n", c);

    close(fd);

    
    fd = open(HOLE_LSEEK, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if(fd == -1) {
        printf("error open %s\n", HOLE_LSEEK);
        exit(1);
    }
    if(lseek(fd, 4096, SEEK_SET)==-1){
        perror("lseek");
    }
    if(write(fd, buffer, 4096)==-1){
        perror("write");
    }

    close(fd);

    
    //空洞区域 du -sh 不计入， ls计入
    //数据会写入空洞中
    fd = open(HOLE_PREALLOC, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
    if(fd == -1) {
        printf("error open %s\n", HOLE_PREALLOC);
        exit(1);
    }
    if(fallocate(fd, 0, 0, 4096)<0) {
        perror("fallocate");
    }
    if(write(fd, buffer, 4096)==-1){
        perror("write");
    }
    
    close(fd);
    
}

