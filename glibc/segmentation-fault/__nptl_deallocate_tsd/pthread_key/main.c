#include <pthread.h>
#include <stdio.h>
#include <malloc.h>

pthread_key_t key;
pthread_t thid1;
pthread_t thid2;

#ifndef MALLOC_KEY
#define MALLOC_KEY  0
#endif

void* thread2(void* arg)
{
    printf("thread:%lu is running\n", pthread_self());
#if MALLOC_KEY    
    int *key_va = malloc(sizeof(int)) ;
    *key_va = 2;
#else
    int key_va = 2;
#endif
    pthread_setspecific(key, (void*)key_va);
    
    printf("thread:%lu return %d\n", pthread_self(), (int)pthread_getspecific(key));
}


void* thread1(void* arg)
{
    printf("thread:%lu is running\n", pthread_self());

#if MALLOC_KEY    
    int *key_va = malloc(sizeof(int)) ;
    *key_va = 1;
#else
    int key_va = 1;
#endif
    
    pthread_setspecific(key, (void*)key_va);

    pthread_create(&thid2, NULL, thread2, NULL);

    printf("thread:%lu return %d\n", pthread_self(), (int)pthread_getspecific(key));
}


int main()
{
    printf("main thread:%lu is running\n", pthread_self());

    //如果 pthread_setspecific 传入的是局部变量，
    //并且 pthread_key_create 传入了析构函数，
    //那么将产生如下段错误
    //#0  __GI___libc_free (mem=0x5) at malloc.c:2941
    //#1  0x00007feb4c550c62 in __nptl_deallocate_tsd () at pthread_create.c:155
    //#2  0x00007feb4c550e73 in start_thread (arg=0x7feb4c17a700) at pthread_create.c:314
    //#3  0x00007feb4c27988d in clone () at ../sysdeps/unix/sysv/linux/x86_64/clone.S:111
    pthread_key_create(&key, free); //这里会段错误 

    pthread_create(&thid1, NULL, thread1, NULL);

    pthread_join(thid1, NULL);
    pthread_join(thid2, NULL);

#if MALLOC_KEY    
    int *key_va = malloc(sizeof(int)) ;
    *key_va = 2;
#else
    int key_va = 2;
#endif
    pthread_setspecific(key, (void*)key_va);
    
    printf("thread:%lu return %d\n", pthread_self(), (int)pthread_getspecific(key));

    pthread_key_delete(key);
        
    printf("main thread exit\n");
    return 0;
}


