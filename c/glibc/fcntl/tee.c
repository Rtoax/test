/* Copyright (c) Colorado School of Mines, .*/
/* All rights reserved.                       */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#define __USE_GNU
#include <fcntl.h>

/*
荣涛 2020.12.15

    stdout          [0]                   [1]
        |  splice   +-----------------------+   splice
        +----+------|   pipefd_stdout       |<--------------stdin
             |      +-----------------------+
             |      
             |      
             |tee
             |
             |      [0]                   [1]
             |      +-----------------------+   splice
             +----->|   pipefd_file         |--------------> filefd = open("file.txt", ...);
                    +-----------------------+
*/


int main(int argc,const char* argv[]){
    if(argc<2){
        printf("usage:%s <file>\n",argv[0]);
        exit(-1);
    }
    
    int filefd=open(argv[1],O_WRONLY|O_CREAT|O_TRUNC,0666);
    assert(filefd>=0);
    
    int pipefd_stdout[2];
    int ret=pipe(pipefd_stdout);
    assert(ret!=-1);
    
    int pipefd_file[2];
    ret=pipe(pipefd_file);
    assert(ret!=-1);
    
    /*将标准输入内容输入管道文件pipefd_stdout*/
    ret=splice(STDIN_FILENO,NULL,pipefd_stdout[1],NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
    assert(ret!=-1);
    
    /*将管道pipefd_stdout[0]输出复制到管道pipefd_file的输入端*/
    ret=tee(pipefd_stdout[0],pipefd_file[1],32768,SPLICE_F_NONBLOCK);
    assert(ret!=-1);
    
    /*将管道pipefd_file的输出定向到文件描述符filefd上*/
    ret=splice(pipefd_file[0],NULL,filefd,NULL,32768,SPLICE_F_MORE|SPLICE_F_MORE);
    assert(ret!=-1);
    
    /*将管道pipefd_stdout的输出定向到标准输出,其内容与文件中完全一致*/
    ret=splice(pipefd_stdout[0],NULL,STDOUT_FILENO,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
    assert(ret!=-1);
    
    close(filefd);
    close(pipefd_file[0]);
    close(pipefd_file[1]);
    close(pipefd_stdout[0]);
    close(pipefd_stdout[1]);
    
    return 0;
}
