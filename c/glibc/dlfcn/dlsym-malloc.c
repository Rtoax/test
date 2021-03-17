#include <stdio.h>

#define __USE_GNU
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>




#ifndef weak_alias
#define weak_alias(name, aliasname) extern typeof (name) aliasname __attribute__ ((weak, alias(#name)))
#endif
#ifndef strong_alias
#define strong_alias(name, aliasname)  extern __typeof (name) aliasname __attribute__ ((alias (#name)))
#endif

#define log_debug(str) do {\
        write(2, str, sizeof(str)); \
    } while(0)

typedef void *(*libc_malloc_fn_t)(size_t size);
typedef void (*libc_free_fn_t)(void *ptr);
typedef void *(*libc_calloc_fn_t)(size_t nmemb, size_t size);
typedef void *(*libc_realloc_fn_t)(void *ptr, size_t size);


static libc_malloc_fn_t g_sys_malloc_func = NULL;
static libc_free_fn_t g_sys_free_func = NULL;
static libc_calloc_fn_t g_sys_calloc_func = NULL;
static libc_realloc_fn_t g_sys_realloc_func = NULL;

#define HOOK_SYS_FUNC(name) \
    if( !g_sys_##name##_func ) { \
        g_sys_##name##_func = (libc_##name##_fn_t)dlsym(RTLD_NEXT,#name);\
    }
    


static libc_malloc_fn_t g_tc_malloc_func = NULL;
static libc_free_fn_t g_tc_free_func = NULL;
static libc_calloc_fn_t g_tc_calloc_func = NULL;
static libc_realloc_fn_t g_tc_realloc_func = NULL;

#define HOOK_TC_FUNC(name, dl) \
    if( !g_tc_##name##_func ) { \
        g_tc_##name##_func = (libc_##name##_fn_t)dlsym(dl,#name);\
        if(!g_tc_##name##_func) { \
            log_debug("Warning: failed to find "#name" in gperftools(libtcmalloc.so).\n"); \
        }\
    }



static libc_malloc_fn_t g_libc_real_malloc_func = NULL;
static libc_free_fn_t g_libc_real_free_func = NULL;
static libc_calloc_fn_t g_libc_real_calloc_func = NULL;
static libc_realloc_fn_t g_libc_real_realloc_func = NULL;


#define HOOK_LIBC_FUNC(name, dl) \
    if( !g_libc_real_##name##_func ) { \
        g_libc_real_##name##_func = (libc_##name##_fn_t)dlsym(dl,#name);\
        if(!g_libc_real_##name##_func) { \
            log_debug("Warning: failed to find "#name" in libc.\n"); \
        }\
    }

static void __real_libc_malloc(void)
{
#define LIBC_SO "libc.so.6"//"/usr/lib64/libc.so.6"

    log_debug("load libc real malloc start.\n");

    void *libc = dlopen(LIBC_SO, RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
    if (!libc) {
        log_debug("[WARN] failed to find libc\n");
        return;
    }
    HOOK_LIBC_FUNC(malloc, libc);
    HOOK_LIBC_FUNC(free, libc);
    HOOK_LIBC_FUNC(calloc, libc);
    HOOK_LIBC_FUNC(realloc, libc);
    
    log_debug("load libc real malloc done.\n");
}

static void __real_tc_malloc(void)
{
#define TC_SO "libtcmalloc.so"//"/usr/lib64/libtcmalloc.so"

    log_debug("load gperftools(libtcmalloc.so) real malloc start.\n");

    void *tc = dlopen(TC_SO, RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
    if (!tc) {
        log_debug("[WARN] failed to find gperftools(libtcmalloc.so)\n");
        return;
    }
    HOOK_TC_FUNC(malloc, tc);
    HOOK_TC_FUNC(free, tc);
    HOOK_TC_FUNC(calloc, tc);
    HOOK_TC_FUNC(realloc, tc);
    
    log_debug("load gperftools(libtcmalloc.so) real malloc done.\n");
}


void __attribute__((constructor(101))) __dlsym_sys_func_init()
{
    log_debug("Memory allocator redirect start.\n");

    
    HOOK_SYS_FUNC(malloc);
    HOOK_SYS_FUNC(free);
    HOOK_SYS_FUNC(calloc);
    HOOK_SYS_FUNC(realloc);
    
    __real_libc_malloc();
    __real_tc_malloc();
    
    log_debug("Memory allocator redirect done.\n");
}

void *malloc(size_t size)
{
    if(g_sys_malloc_func) {
        log_debug("g_sys_malloc_func called.\n");
        return g_sys_malloc_func(size);
    }
    if(g_libc_real_malloc_func) {
        log_debug("g_libc_real_malloc_func called.\n");
        return g_libc_real_malloc_func(size);
    }
    log_debug("[ERROR] malloc null.\n");
    return NULL;
}


void free(void *ptr)
{
    if(g_sys_free_func) {
        log_debug("g_sys_free_func called.\n");
        g_sys_free_func(ptr);
        return ;
    }
    if(g_libc_real_free_func) {
        log_debug("g_libc_real_free_func called.\n");
        g_libc_real_free_func(ptr);
        return ;
    }
    log_debug("[ERROR] free null.\n");
    return ;
}

void *calloc(size_t nmemb, size_t size)
{
    if(g_sys_calloc_func) {
        log_debug("g_sys_calloc_func called.\n");
        return g_sys_calloc_func(nmemb, size);
    }
    if(g_libc_real_calloc_func) {
        log_debug("g_libc_real_calloc_func called.\n");
        return g_libc_real_calloc_func(nmemb, size);
    }
    log_debug("[ERROR] calloc null.\n");
    return NULL;
}

void *realloc(void *ptr, size_t size)
{
    if(g_sys_realloc_func) {
        log_debug("g_sys_realloc_func called.\n");
        return g_sys_realloc_func(ptr, size);
    }
    if(g_libc_real_realloc_func) {
        log_debug("g_libc_real_realloc_func called.\n");
        return g_libc_real_realloc_func(ptr, size);
    }
    log_debug("[ERROR] realloc null.\n");
    return NULL;
}

#if 0
void *tc_malloc(size_t size)
{
    if(g_tc_malloc_func) {
        log_debug("g_tc_malloc_func called.\n");
        return g_tc_malloc_func(size);
    }
    if(g_sys_malloc_func) {
        log_debug("g_sys_malloc_func called.\n");
        return g_sys_malloc_func(size);
    }
    if(g_libc_real_malloc_func) {
        log_debug("g_libc_real_malloc_func called.\n");
        return g_libc_real_malloc_func(size);
    }
    log_debug("[ERROR] malloc null.\n");
    return NULL;
}


void tc_free(void *ptr)
{
    if(g_tc_free_func) {
        log_debug("g_tc_free_func called.\n");
        g_tc_free_func(ptr);
        return ;
    }
    if(g_sys_free_func) {
        log_debug("g_sys_free_func called.\n");
        g_sys_free_func(ptr);
        return ;
    }
    if(g_libc_real_free_func) {
        log_debug("g_libc_real_free_func called.\n");
        g_libc_real_free_func(ptr);
        return ;
    }
    log_debug("[ERROR] free null.\n");
    return ;
}

void *tc_calloc(size_t nmemb, size_t size)
{
    if(g_tc_calloc_func) {
        log_debug("g_tc_calloc_func called.\n");
        return g_tc_calloc_func(nmemb, size);
    }
    if(g_sys_calloc_func) {
        log_debug("g_sys_calloc_func called.\n");
        return g_sys_calloc_func(nmemb, size);
    }
    if(g_libc_real_calloc_func) {
        log_debug("g_libc_real_calloc_func called.\n");
        return g_libc_real_calloc_func(nmemb, size);
    }
    log_debug("[ERROR] calloc null.\n");
    return NULL;
}

void *tc_realloc(void *ptr, size_t size)
{
    if(g_tc_realloc_func) {
        log_debug("g_tc_realloc_func called.\n");
        return g_tc_realloc_func(ptr, size);
    }
    if(g_sys_realloc_func) {
        log_debug("g_sys_realloc_func called.\n");
        return g_sys_realloc_func(ptr, size);
    }
    if(g_libc_real_realloc_func) {
        log_debug("g_libc_real_realloc_func called.\n");
        return g_libc_real_realloc_func(ptr, size);
    }
    log_debug("[ERROR] realloc null.\n");
    return NULL;
}
#endif

#ifndef NOTEST
#define TEST
#endif

#ifdef TEST
int main() 
{
    printf(">>>>>>>>>>>>>>>>>>>>>> main start <<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    
    char *str1 = malloc(64);
    str1 = realloc(str1, 128);

    sprintf(str1, "Hello World.");
    printf("str1 = %s\n", str1);

    free(str1);
}
#endif
