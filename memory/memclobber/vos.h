#ifndef __VOS_H
#define __VOS_H 1

#include <sys/types.h>
/**
 *  
 */

#define vos_malloc(size) __canhook__malloc(0, size, __func__, __LINE__)
#define vos_free(ptr) __canhook__free(0, ptr, __func__, __LINE__)
#define vos_calloc(nmemb, size) __canhook__calloc(0, nmemb, size, __func__, __LINE__)
#define vos_realloc(ptr, size) __canhook__realloc(0, ptr, size, __func__, __LINE__)
#define vos_strdup(s) __canhook__strdup(0, s, __func__, __LINE__)
#define vos_strndup(s, n) __canhook__strndup(0, s, n, __func__, __LINE__)



void *__canhook__malloc(unsigned long idx, size_t size, const char* _func, const int _line);
void __canhook__free(unsigned long idx, void *ptr, const char* _func, const int _line);
void *__canhook__calloc(unsigned long idx, size_t nmemb, size_t size, const char* _func, const int _line);
void *__canhook__realloc(unsigned long idx, void *ptr, size_t size, const char* _func, const int _line);


char *__canhook__strdup(unsigned long idx, const char *s, const char* _func, const int _line);
char *__canhook__strndup(unsigned long idx, const char *s, size_t n, const char* _func, const int _line);



#endif /*<__VOS_H>*/

