#ifndef __x86_64__
# error "Not support Your Arch, just support x86-64"
#endif


#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <unistd.h>
    
#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <assert.h>
#include <memory.h>
#include <pthread.h>
#include <syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define gettid() syscall(__NR_gettid)

#define log_enqueue(fmt...)  do{printf("\033[33m[%d]", gettid());printf(fmt);printf("\033[m");}while(0)
#define log_dequeue(fmt...)  do{printf("\033[32m[%d]", gettid());printf(fmt);printf("\033[m");}while(0)

#ifndef RDTSC
#define RDTSC() ({\
    register uint32_t a,d; \
    __asm__ __volatile__( "rdtsc" : "=a"(a), "=d"(d)); \
    (((uint64_t)a)+(((uint64_t)d)<<32)); \
    })
#endif


/* Both Process and CPU ids should be positive numbers. */
static int __convert_str_to_int(char* begin)
{
    if (!begin)
    {
        errx(1, "Invalid arguments for %s", __func__);
    }

    errno = 0;
    char *end = NULL;
    long num = strtol(begin, &end, 10);
    if (errno || (*end != '\0') || (num > INT_MAX) || (num < 0))
    {
        errx(1, "Invalid integer: %s", begin);
    }
    return (int)num;
}

/*
 * The cpu list should like 1-3,6
 */
static void __parse_cpu_list(char* cpu_list, cpu_set_t* cpu_set)
{
    if (!cpu_list || !cpu_set)
    {
        errx(1, "Invalid arguments for %s", __func__);
    }
	int i;
    char* begin = cpu_list;
    while (1)
    {
        bool last_token = false;
        char* end = strchr(begin, ',');
        if (!end)
        {
            last_token = true;
        }
        else
        {
            *end = '\0';
        }

        char* hyphen = strchr(begin, '-');
        if (hyphen)
        {
            *hyphen = '\0';
            int first_cpu = __convert_str_to_int(begin);
            int last_cpu = __convert_str_to_int(hyphen + 1);
            if ((first_cpu > last_cpu) || (last_cpu >= CPU_SETSIZE))
            {
                errx(1, "Invalid cpu list: %s", cpu_list);
            }
            for (i = first_cpu; i <= last_cpu; i++)
            {
                CPU_SET(i, cpu_set);
            }
        }
        else
        {
            CPU_SET(__convert_str_to_int(begin), cpu_set);
        }

        if (last_token)
        {
            break;
        }
        else
        {
            begin = end + 1;
        }
    }
}


//my_taskset(gettid(), (char*)(ULONG)ulArg1);
static void reset_self_cpuset(char *cpu_list)
{
    printf("[CPUSET] pid %d, cpu_list: %s\n", gettid(), cpu_list);
    int pid = gettid();
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    __parse_cpu_list(cpu_list, &cpu_set);
    if (0 != sched_setaffinity(pid, sizeof(cpu_set_t), &cpu_set)) {
        printf("[ERROR]: sched_setaffinity(%d, %d, %s) ", 
                    gettid(), sizeof(cpu_set_t), cpu_list);
        assert(0);
    }
}

