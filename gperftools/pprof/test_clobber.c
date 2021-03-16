#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

void* test_task_fn(void* unused)
{
	printf("test_task_fn.\n");

//          |       str_pad1            |   str   |         str_pad2          |
//          +---------------------------+---------+---------------------------+
    
    
//    |     |       str_pad1            |     |   str   |     |         str_pad2          |     |
//    +#####+---------------------------+#####+---------+#####+---------------------------+#####+


    char *str_pad1 = malloc(64);
    char *str = malloc(64);
    char *str_pad2 = malloc(6400);
    
    str_pad1[63] = 'Z';
    str_pad2[0] = 'Z';
    
#ifdef CLOBBER
    str[-65] = 'A'; //检测不到
    str[-64] = 'A'; //能检测到
//    str[-1] = 'A';  //能检测到
//    str[64] = 'A';  //能检测到
    str[127] = 'A'; //能检测到
    str[128] = 'A'; //检测不到
#else
    str[0] = 'A';
    str[63] = 'A';
#endif

    printf("%c\n", str_pad1[63]);
    printf("%c\n", str_pad2[0]);
    
    free(str);
    free(str_pad1);
    free(str_pad2);
    
    pthread_exit(NULL);
}

/* The main program. */
int main ()
{
	pthread_t thread_id;
    
	pthread_create(&thread_id, NULL, test_task_fn, NULL);

	pthread_join(thread_id, NULL);

    printf("Exit program.\n");

	return 0;
}

