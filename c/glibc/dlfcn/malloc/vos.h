#ifndef __VOS_H
#define __VOS_H 1

#include <sys/types.h>
/**
 *  
 */

#define vos_malloc(size) __vos_malloc(size, __func__, __LINE__)
#define vos_free(ptr) __vos_free(ptr, __func__, __LINE__)
#define vos_calloc(nmemb, size) __vos_calloc(nmemb, size, __func__, __LINE__)
#define vos_realloc(ptr, size) __vos_realloc(ptr, size, __func__, __LINE__)
#define vos_strdup(s) __vos_strdup(s, __func__, __LINE__)
#define vos_strndup(s, n) __vos_strndup(s, n, __func__, __LINE__)



void *__vos_malloc(size_t size, const char* _func, const int _line);
void __vos_free(void *ptr, const char* _func, const int _line);
void *__vos_calloc(size_t nmemb, size_t size, const char* _func, const int _line);
void *__vos_realloc(void *ptr, size_t size, const char* _func, const int _line);


char *__vos_strdup(const char *s, const char* _func, const int _line);
char *__vos_strndup(const char *s, size_t n, const char* _func, const int _line);



#endif /*<__VOS_H>*/
