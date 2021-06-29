#include <stdio.h>
#include <syscall.h>

enum {
    MODULE_1,
#define      MODULE_1_NAME  "module1"   
#define      TASK_1_NAME  "task1"  
    MODULE_2,
#define      MODULE_2_NAME  "module2" 
#define      TASK_2_NAME  "task2"   
    MODULE_3,
#define      MODULE_3_NAME  "module3"  
#define      TASK_3_NAME  "task3"  
    MODULE_NUM,
};
#define MODULE(e) e##_NAME

const static char __attribute__((unused)) *modules_tasks_name[] = {
    TASK_1_NAME,
    TASK_2_NAME,
    TASK_3_NAME,
};


struct task_arg {
    int cpu;
    int module;
    FILE *fp_ofile;
};

#define gettid() syscall(__NR_gettid)


extern pthread_t module_threads[MODULE_NUM];


int set_thread_cpu_affinity(int i);


void test_benchmark(int nthreads, char *ofile);
void modules_init();
void modules_loop();

