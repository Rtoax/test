#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

#include "common.h"

void* test_clobber_task_fn(void* unused)
{
	printf("test_task_fn.\n");

    char *str_pad1 = malloc(64);
    char *str = malloc(64);
    char *str_pad2 = malloc(64);
    
    str[-65] = 'A'; //检测不到
    str[-64] = 'A'; //能检测到
    str[-1] = 'A';  //能检测到
    str[64] = 'A';  //能检测到
    str[127] = 'A'; //能检测到
    str[128] = 'A'; //检测不到
    
    free(str);
    free(str_pad1);
    free(str_pad2);
    
    pthread_exit(NULL);
}



/* The main program. */
int main (int argc, char *argv[])
{
    int nr_tasks = parse_arg_nr_tasks(argc, argv);
    create_tasks(nr_tasks, test_clobber_task_fn, NULL);

    printf("Exit program.\n");

	return 0;
}

