#include <stdio.h>

#define __USE_GNU
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <memtools.h>


#ifdef NODEBUG
#define log_debug(str) do {} while(0)
#else
#define log_debug(str) do {\
        write(2, str, sizeof(str)); \
    } while(0)
#endif



static malloc_fn_t      g_sys_malloc_func = NULL;
static free_fn_t        g_sys_free_func = NULL;
static calloc_fn_t      g_sys_calloc_func = NULL;
static realloc_fn_t     g_sys_realloc_func = NULL;
static strdup_fn_t      g_sys_strdup_func = NULL;
static strndup_fn_t     g_sys_strndup_func = NULL;


static malloc_fn_t      g_libc_real_malloc_func = NULL;
static free_fn_t        g_libc_real_free_func = NULL;
static calloc_fn_t      g_libc_real_calloc_func = NULL;
static realloc_fn_t     g_libc_real_realloc_func = NULL;
static strdup_fn_t      g_libc_real_strdup_func = NULL;
static strndup_fn_t     g_libc_real_strndup_func = NULL;



#define HOOK_SYS_FUNC(name) \
    if( !g_sys_##name##_func ) { \
        g_sys_##name##_func = (name##_fn_t)dlsym(RTLD_NEXT,#name);\
        if(!g_sys_##name##_func) { \
            log_debug("Warning: failed to find "#name" in sys_RTLD_NEXT.\n"); \
        }\
    }

#define HOOK_LIBC_FUNC(name, dl) \
    if( !g_libc_real_##name##_func ) { \
        g_libc_real_##name##_func = (name##_fn_t)dlsym(dl,#name);\
        if(!g_libc_real_##name##_func) { \
            log_debug("Warning: failed to find "#name" in "LIBC_SO".\n"); \
        }\
    }
    

    
static void __real_sys_malloc(void)
{
    HOOK_SYS_FUNC(malloc);
    HOOK_SYS_FUNC(free);
    HOOK_SYS_FUNC(calloc);
    HOOK_SYS_FUNC(realloc);
    HOOK_SYS_FUNC(strdup);
    HOOK_SYS_FUNC(strndup);
}

static void __real_libc_malloc(void)
{

    log_debug("load libc real malloc start.\n");

    void *libc = dlopen(LIBC_SO, RTLD_LAZY | RTLD_LOCAL | RTLD_NOLOAD);
    if (!libc) {
        log_debug("[WARN] failed to find "LIBC_SO"\n");
        return;
    }
    HOOK_LIBC_FUNC(malloc, libc);
    HOOK_LIBC_FUNC(free, libc);
    HOOK_LIBC_FUNC(calloc, libc);
    HOOK_LIBC_FUNC(realloc, libc);
    HOOK_LIBC_FUNC(strdup, libc);
    HOOK_LIBC_FUNC(strndup, libc);
    
    log_debug("load "LIBC_SO" real malloc done.\n");
}


void __memhook_init()
{
    log_debug("Memory allocator redirect start.\n");

    __real_sys_malloc();
    __real_libc_malloc();
    
    log_debug("Memory allocator redirect done.\n");
}

void *malloc(size_t size)
{
    if(g_sys_malloc_func) {
        return g_sys_malloc_func(size);
    }
    if(g_libc_real_malloc_func) {
        return g_libc_real_malloc_func(size);
    }
    log_debug("[ERROR] malloc null.\n");
    return NULL;
}


void free(void *ptr)
{
    if(g_sys_free_func) {
        g_sys_free_func(ptr);
        return ;
    }
    if(g_libc_real_free_func) {
        g_libc_real_free_func(ptr);
        return ;
    }
    log_debug("[ERROR] free null.\n");
    return ;
}

void *calloc(size_t nmemb, size_t size)
{
    if(g_sys_calloc_func) {
        return g_sys_calloc_func(nmemb, size);
    }
    if(g_libc_real_calloc_func) {
        return g_libc_real_calloc_func(nmemb, size);
    }
    log_debug("[ERROR] calloc null.\n");
    return NULL;
}

void *realloc(void *ptr, size_t size)
{
    if(g_sys_realloc_func) {
        return g_sys_realloc_func(ptr, size);
    }
    if(g_libc_real_realloc_func) {
        return g_libc_real_realloc_func(ptr, size);
    }
    log_debug("[ERROR] realloc null.\n");
    return NULL;
}

char *strdup(const char *s) 
{
    if(g_sys_strdup_func) {
        return g_sys_strdup_func(s);
    }
    if(g_libc_real_strdup_func) {
        return g_libc_real_strdup_func(s);
    }
    log_debug("[ERROR] strdup null.\n");
    return NULL;
}

char *strndup(const char *s, size_t n)
{
    if(g_sys_strndup_func) {
        return g_sys_strndup_func(s, n);
    }
    if(g_libc_real_strndup_func) {
        return g_libc_real_strndup_func(s, n);
    }
    log_debug("[ERROR] strndup null.\n");
    return NULL;
    
}

