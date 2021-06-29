#include <syscall.h>

enum {
    MODULE_1,
#define      MODULE_1_NAME  "module1"   
    MODULE_2,
#define      MODULE_2_NAME  "module2"  
    MODULE_3,
#define      MODULE_3_NAME  "module3"  
    MODULE_NUM,
};
#define MODULE(e) e##_NAME

const static char __attribute__((unused)) *modules_name[] = {
    MODULE_1_NAME,
    MODULE_2_NAME,
    MODULE_3_NAME,
};


struct task_arg {
    int cpu;
    int module;
};

#define gettid() syscall(__NR_gettid)


extern pthread_t module_threads[MODULE_NUM];


int set_thread_cpu_affinity(int i);


void test_benchmark(int nthreads);
void modules_init();
void modules_loop();

