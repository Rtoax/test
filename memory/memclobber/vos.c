#include <vos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef VOS_DEBUG
#define vos_debug(f, l, fmt...) do {\
        fprintf(stderr, "[VOS %s:%d call %s]\n", f, l, __func__); \
        fprintf(stderr, fmt);   \
    }while(0)
#else
#define vos_debug(f, l, fmt...) do {}while(0)
#endif

void *__canhook__malloc(unsigned long idx, size_t size, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return malloc(size);
}


void __canhook__free(unsigned long idx, void *ptr, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    free(ptr);
}


void *__canhook__calloc(unsigned long idx, size_t nmemb, size_t size, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return calloc(nmemb, size);
}


void *__canhook__realloc(unsigned long idx, void *ptr, size_t size, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return realloc(ptr, size);
}


char *__canhook__strdup(unsigned long idx, const char *s, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return strdup(s);
}

char *__canhook__strndup(unsigned long idx, const char *s, size_t n, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return strndup(s, n);
}



