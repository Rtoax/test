#include <syscall.h>


struct task_arg {
    int cpu;
};

#define gettid() syscall(__NR_gettid)


int set_thread_cpu_affinity(int i);

