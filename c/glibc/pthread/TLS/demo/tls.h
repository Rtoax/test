//Tls.h
#ifndef __SAP_UTIL_TLS_H_
#define __SAP_UTIL_TLS_H_
#include <pthread.h>
 
class Tls
{
public:
    static int pthread_atexit_add(void *arg, void (*free_fn)(void *) );
    static int pthread_atexit_remove(void *arg, void (*free_fn)(void*) );
protected:
    static void pthread_atexit_done(void *arg);
    static void pthread_atexit_init(void);
protected:
    static pthread_key_t    _pthread_atexit_key;
    static pthread_once_t   _pthread_atexit_control_once;
};
#endif

