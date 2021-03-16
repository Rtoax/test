#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

#define LEAK

#ifdef LEAK
#define free(a) do{}while(0)
#else
#warning LEAK macro ready to set.
#endif

void* test_task_fn(void* unused)
{
	printf("test_task_fn.\n");
    char *str;
    str = malloc(64);
	free(str);

    int i;
    for (i=0; i<10; i++) {
        str = malloc(64);
		printf("leak 64 bytes. i = %d\n", i);
		free(str);
    }

    pthread_exit(NULL);
	return NULL;
}

#define NR_TASKS 10

/* The main program. */
int main ()
{
    int i;
	pthread_t tasks[NR_TASKS];

    for(i=0;i<NR_TASKS;i++)
	    pthread_create(&tasks[i], NULL, test_task_fn, NULL);

    for(i=0;i<NR_TASKS;i++)
    	pthread_join(tasks[i], NULL);

    printf("Exit program.\n");

	return 0;
}

