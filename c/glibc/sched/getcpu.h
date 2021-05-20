#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>

int getcpu(unsigned int *cpu, unsigned int *node);
