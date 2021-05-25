#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

#include "common.h"


void* test_leak_task_fn(void* unused)
{
	printf("test_task_fn.\n");
    
    char *str = malloc(64);

    int i;
    for (i=0; i<2; i++) {
        str = malloc(64);
    }


	sleep(20);
    free(str);

    pthread_exit(NULL);
	return NULL;
}



/* The main program. */
int main(int argc, char *argv[])
{
    int nr_tasks = parse_arg_nr_tasks(argc, argv);
    create_tasks(nr_tasks, test_leak_task_fn, NULL);

    printf("Exit program.\n");

	return 0;
}

