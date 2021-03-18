
#ifndef NR_TASKS
#define NR_TASKS    4
#endif

#define ARG_NR_TASKS_NAME   "ntasks"


static int parse_arg_nr_tasks(int argc, char *argv[])
{   
    int i, nr_tasks = NR_TASKS;
    for(i=0;i<argc;i++) {
        if(strncmp(argv[i], ARG_NR_TASKS_NAME, strlen(ARG_NR_TASKS_NAME)) == 0) {
            int N = argv[i+1]?atoi(argv[i+1]):NR_TASKS;
            nr_tasks = N?N:nr_tasks;
        }
    }
    return nr_tasks;
}


static void create_tasks(int nr_tasks, void* (*callback)(void* arg), void *arg)
{
    int i;
	pthread_t tasks[nr_tasks];

    for(i=0;i<nr_tasks;i++) {
	    pthread_create(&tasks[i], NULL, callback, arg);
        
    }
    for(i=0;i<nr_tasks;i++) {
	    pthread_join(tasks[i], NULL);
    }
    
}




