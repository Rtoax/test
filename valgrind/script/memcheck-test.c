#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
/**
 *  
 */
#ifdef VOS_DEBUG
#define vos_debug(f, l, fmt...) do {\
        fprintf(stderr, "[%s:%d call %s]\n", f, l, __func__); \
        fprintf(stderr, fmt);   \
    }while(0)
#else
#define vos_debug(f, l, fmt...) do {}while(0)
#endif

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



void *__vos_malloc(size_t size, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return malloc(size);
}


void __vos_free(void *ptr, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    free(ptr);
}


void *__vos_calloc(size_t nmemb, size_t size, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return calloc(nmemb, size);
}


void *__vos_realloc(void *ptr, size_t size, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return realloc(ptr, size);
}


char *__vos_strdup(const char *s, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return strdup(s);
}

char *__vos_strndup(const char *s, size_t n, const char* _func, const int _line)
{
    vos_debug(_func, _line, "");
    return strndup(s, n);
}



void demo_test1() {
    
    char *str1 = vos_malloc(64);
    char *str2 = vos_malloc(64);

    static int N = 0;

    if((++N)%3==0) {
        str2[-1] = 'A';
        str2[64] = 'A';
    }
    
    str1 = vos_realloc(str1, 128);

    sprintf(str1, "Hello World.");
    printf("str1 = %s\n", str1);
    char *str3 = vos_strdup(str1);
    printf("str3 = %s\n", str3);
    vos_free(str3);
    
    vos_free(str1);
    str1 = vos_malloc(128);

    vos_free(str1);
    vos_free(str2);
}

void* test_task_fn(void* unused)
{
	printf("test_task_fn.\n");
    
    int i, j=0;
    while(1) {
        printf("while loop -> %d.\n", ++j);
        demo_test1();
        sleep(1);
    }

    pthread_exit(NULL);
	return NULL;
}

/* The main program. */
int main(int argc, char *argv[]) 
{
    int i;
    for(i=0;i<argc;i++){
        printf("argv[%d] = %s\n", i, argv[i]);
    }

	pthread_t thread_id;
    
	pthread_create(&thread_id, NULL, test_task_fn, NULL);

	pthread_join(thread_id, NULL);

    printf("Exit program.\n");

	return 0;
}



