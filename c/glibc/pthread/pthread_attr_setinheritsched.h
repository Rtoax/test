//pthread_attr_setinheritsched,  pthread_attr_getinheritsched  -  set/get  inherit-scheduler  attribute  in
//thread attributes object
//
//SYNOPSIS
#include <pthread.h>

/* Scheduler inheritance.  */
enum
{
  PTHREAD_INHERIT_SCHED,    //继承父进程的调度策略
#define PTHREAD_INHERIT_SCHED   PTHREAD_INHERIT_SCHED
  PTHREAD_EXPLICIT_SCHED    //不继承，只有不继承父线程的调度策略才可以设置线程的调度策略
#define PTHREAD_EXPLICIT_SCHED  PTHREAD_EXPLICIT_SCHED
};


//设置线程是否继承父线程调度策略
int pthread_attr_setinheritsched(pthread_attr_t *attr,
                                int inheritsched);
int pthread_attr_getinheritsched(pthread_attr_t *attr,
                                int *inheritsched);

//Compile and link with -pthread.

