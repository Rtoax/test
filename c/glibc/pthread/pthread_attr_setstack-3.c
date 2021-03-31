#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<pthread.h>
#include<string.h>
#include<errno.h>

#include "../../memshow.c"

#ifndef PTHREAD_STACK_MIN 
#define PTHREAD_STACK_MIN 16384
#endif

void do_something(int i) {
    char str[] = {"1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"};
    char I[] = {1,2,3,4,5,6,7,8,9,0,11,12,13,14,15,16};
    memset(str, 'a'+i, sizeof(str));
    if(i<=0) return;
//    printf("thread run\n");
    do_something(i-1);
}

void *thread_routine(void *arg)
{
    do_something(10);
    while(1) {
        usleep(1000);
        do_something(10);
    }
    return NULL;
}

int main(void)
{
    int ret;
    void *stackAddr = NULL;
    
    pthread_t tid;
    pthread_attr_t attr;
    int paseSize = getpagesize();
    size_t stacksize = paseSize<PTHREAD_STACK_MIN?PTHREAD_STACK_MIN:paseSize;
    
    ret = posix_memalign(&stackAddr, paseSize, stacksize);
    if(0 != ret) {
        printf("posix_memalign failed, errno:%s\n", strerror(ret));
        return -1;
    }
    pthread_attr_init(&attr);

    ret = pthread_attr_setstack(&attr,stackAddr,stacksize);
    if(0 != ret) {
        printf("pthread_attr_setstack failed, errno:%s\n", strerror(ret));
        return -1;
    }
    
    pthread_create(&tid,&attr,thread_routine,NULL);

    while(1) {
        sleep(1);
        char *__stack;
        size_t __stacksize;
        pthread_attr_getstack(&attr,(void*)&__stack, &__stacksize);
        printf(">>>>>>>>>>>>>>stacksize:%u\n",__stacksize);
        memshow("[Stack]", __stack, __stacksize, 0);
    }
    sleep(2);
    return 0;
}


