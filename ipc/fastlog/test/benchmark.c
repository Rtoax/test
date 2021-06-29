#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <signal.h>

#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

#include <fastlog.h>

#include "common.h"

struct benchmark {
    const char *intro;
    int (*test_func)(unsigned long total_dequeue);
};


int benchmark_staticstring(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "staticstring", "Static String : critical\n");
    FAST_LOG(FASTLOG_WARNING,   "staticstring", "Static String : warning\n");
    FAST_LOG(FASTLOG_ERR,       "staticstring", "Static String : error\n");
    FAST_LOG(FASTLOG_INFO,      "staticstring", "Static String : information\n");
    FAST_LOG(FASTLOG_DEBUG,     "staticstring", "Static String : debug\n");

    return 5;
}

int benchmark_singleint(unsigned long total_dequeue)
{
    int i = total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "singleint", "Single Int : critical    %d\n", i);
    FAST_LOG(FASTLOG_WARNING,   "singleint", "Single Int : warning     %d\n", i);
    FAST_LOG(FASTLOG_ERR,       "singleint", "Single Int : error       %d\n", i);
    FAST_LOG(FASTLOG_INFO,      "singleint", "Single Int : information %d\n", i);
    FAST_LOG(FASTLOG_DEBUG,     "singleint", "Single Int : debug       %d\n", i);

    return 5;
}

int benchmark_twoint(unsigned long total_dequeue)
{
    int i = total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "twoint", "Two Int : critical      %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_WARNING,   "twoint", "Two Int : warning       %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_ERR,       "twoint", "Two Int : error         %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_INFO,      "twoint", "Two Int : information   %d %d\n", i, i+1);
    FAST_LOG(FASTLOG_DEBUG,     "twoint", "Two Int : debug         %d %d\n", i, i+1);

    return 5;
}

int benchmark_threeint(unsigned long total_dequeue)
{
    int i = total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "threeint", "Three Int : critical      %d %d %d\n", i, i+1, i+2);
    FAST_LOG(FASTLOG_WARNING,   "threeint", "Three Int : warning       %d %d %d\n", i, i+1, i+2);
    FAST_LOG(FASTLOG_ERR,       "threeint", "Three Int : error         %d %d %d\n", i, i+1, i+2);
    FAST_LOG(FASTLOG_INFO,      "threeint", "Three Int : information   %d %d %d\n", i, i+1, i+2);
    FAST_LOG(FASTLOG_DEBUG,     "threeint", "Three Int : debug         %d %d %d\n", i, i+1, i+2);

    return 5;
}
int benchmark_fourint(unsigned long total_dequeue)
{
    int i = total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "fourint", "Four Int : critical    %d %d %d %d\n", i, i+1, i+2, i+3);
    FAST_LOG(FASTLOG_WARNING,   "fourint", "Four Int : warning     %d %d %d %d\n", i, i+1, i+2, i+3);
    FAST_LOG(FASTLOG_ERR,       "fourint", "Four Int : error       %d %d %d %d\n", i, i+1, i+2, i+3);
    FAST_LOG(FASTLOG_INFO,      "fourint", "Four Int : information %d %d %d %d\n", i, i+1, i+2, i+3);
    FAST_LOG(FASTLOG_DEBUG,     "fourint", "Four Int : debug       %d %d %d %d\n", i, i+1, i+2, i+3);

    return 5;
}

int benchmark_singlelong(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "singlelong", "Single Long : critical      %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_WARNING,   "singlelong", "Single Long : warning       %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_ERR,       "singlelong", "Single Long : error         %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_INFO,      "singlelong", "Single Long : information   %ld\n", total_dequeue);
    FAST_LOG(FASTLOG_DEBUG,     "singlelong", "Single Long : debug         %ld\n", total_dequeue);

    return 5;
}
int benchmark_twolong(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_CRIT,      "twolong", "Two Long : critical        %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_WARNING,   "twolong", "Two Long : warning         %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_ERR,       "twolong", "Two Long : error           %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_INFO,      "twolong", "Two Long : information     %ld %ld\n", total_dequeue, total_dequeue);
    FAST_LOG(FASTLOG_DEBUG,     "twolong", "Two Long : debug           %ld %ld\n", total_dequeue, total_dequeue);

    return 5;
}
int benchmark_singledouble(unsigned long total_dequeue)
{
    double d = (double)total_dequeue;
    
    FAST_LOG(FASTLOG_CRIT,      "singledouble", "Single Double : critical      %lf\n", d);
    FAST_LOG(FASTLOG_WARNING,   "singledouble", "Single Double : warning       %lf\n", d);
    FAST_LOG(FASTLOG_ERR,       "singledouble", "Single Double : error         %lf\n", d);
    FAST_LOG(FASTLOG_INFO,      "singledouble", "Single Double : information   %lf\n", d);
    FAST_LOG(FASTLOG_DEBUG,     "singledouble", "Single Double : debug         %lf\n", d);

    return 5;
}
int benchmark_twodouble(unsigned long total_dequeue)
{
    double d = (double)total_dequeue;
    FAST_LOG(FASTLOG_CRIT,      "twolong", "Two Double : critical      %lf %lf\n", d, d);
    FAST_LOG(FASTLOG_WARNING,   "twolong", "Two Double : warning       %lf %lf\n", d, d);
    FAST_LOG(FASTLOG_ERR,       "twolong", "Two Double : error         %lf %lf\n", d, d);
    FAST_LOG(FASTLOG_INFO,      "twolong", "Two Double : information   %lf %lf\n", d, d);
    FAST_LOG(FASTLOG_DEBUG,     "twolong", "Two Double : debug         %lf %lf\n", d, d);

    return 5;
}


int benchmark_complex(unsigned long total_dequeue)
{
    FAST_LOG(FASTLOG_WARNING,   "Complex1", "[%s] CPU %d(%d)\n", "Running ON", __fastlog_sched_getcpu(), __fastlog_getcpu());
    FAST_LOG(FASTLOG_ERR,       "Complex1", "I gotta three PI %f %f %f\n", 3.14, 3.14, 3.14);
    FAST_LOG(FASTLOG_CRIT,      "Complex1", "I gotta three PI %2.3f %2.3f %2.3lf\n", 3.14, 3.14, 3.14);
    FAST_LOG(FASTLOG_INFO,      "Complex1", "I gotta a few of Int %d %d %ld %d %d %d %s\n", 1, 2, 3L, 1, 2, 3, "Hello");
    FAST_LOG(FASTLOG_WARNING,   "Complex1", ">>># %d %ld %ld %s %f #<<<\n", 1, 2L, 3L, "Hello", 1024.0);
    
    FAST_LOG(FASTLOG_CRIT,      "Complex2", ">>># Hello %.*ld %.*ld 3#<<<\n", 7, total_dequeue, 7, total_dequeue+1);
    FAST_LOG(FASTLOG_DEBUG,     "Complex2", ">>># Hello %*.*ld 3#<<<\n", 20, 7, total_dequeue+10);
    FAST_LOG(FASTLOG_INFO,      "Complex2", ">>># Hello %*.*ld 3#<<<\n", 30, 7, total_dequeue+20);
    FAST_LOG(FASTLOG_ERR,       "Complex2", ">>># Hello %*s 3#<<<\n", 10, "World");
    FAST_LOG(FASTLOG_WARNING,   "Complex2", ">>># Hello %.*s 3#<<<\n", 4, "World");

    return 10;
}

#define N_Benchmark  (sizeof(benchmarks)/sizeof(benchmarks[0]))    

static struct benchmark benchmarks[] = {
    {"StaticString",    benchmark_staticstring},
    {"SingleInt",       benchmark_singleint},
//    {"TwoInt",          benchmark_twoint},
//    {"ThreeInt",        benchmark_threeint},
//    {"FourInt",         benchmark_fourint},
    {"SingleLong",      benchmark_singlelong},
//    {"TwoLong",         benchmark_twolong},
//    {"SingleDouble",    benchmark_singledouble},
//    {"TwoDouble",       benchmark_twodouble},
    {"Complex",         benchmark_complex},
};


int __thread idx_benchmark = 0;
int __thread (*test_func)(unsigned long total_dequeue);

unsigned long __thread total_dequeue = 0;

static void *task_routine(void*param) 
{
    int i;
    struct task_arg * arg = (struct task_arg *)param;

    if(arg->fp_ofile != stderr) {
        for(i=0; i<N_Benchmark; i++) {
            fprintf(arg->fp_ofile, "%-15s  ", benchmarks[i].intro);
        }
        fprintf(arg->fp_ofile, "\n");
    }
    set_thread_cpu_affinity(arg->cpu);    


    test_func = benchmarks[idx_benchmark].test_func;
    
    
    struct timeval start, end;
    gettimeofday(&start, NULL);

    while(1) {
        
        total_dequeue += test_func(total_dequeue);
        if(total_dequeue % 100000 == 0) {
            

            static unsigned int statistics_count = 0;

            gettimeofday(&end, NULL);

            unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
            double nmsg_per_sec = (double)((total_dequeue)*1.0 / usec) * 1000000;
            
            fprintf(stdout, "\t %-5d \t %-20s \t %-15ld\n", statistics_count,
                            benchmarks[idx_benchmark].intro,
                            (unsigned long )nmsg_per_sec);

            if(arg->fp_ofile != stderr) {
                fprintf(arg->fp_ofile, "%-15ld  ", (unsigned long )nmsg_per_sec);
            }


            statistics_count ++;
            
            total_dequeue = 0;
            idx_benchmark++;
            
            if(idx_benchmark >= N_Benchmark) {
                idx_benchmark = 0;  //重新再来一遍
                fprintf(arg->fp_ofile, "\n");
            }
            
            test_func = benchmarks[idx_benchmark].test_func;

            gettimeofday(&start, NULL);
        }
    }

    
    fprintf(stderr, "exit-----------\n");
    pthread_exit(arg);
}



void test_benchmark(int nthreads, char *ofile)
{
    int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int itask;
    pthread_t threads[64];
    int nthread = nthreads;
    char thread_name[32] = {0};



    printf("Startup %d Threads.\n", nthreads);

    fprintf(stdout, "\t %-5s \t %-20s \t %-15s\n", "NUM", "Benchmark", "(Logs/s)");
    

    for(itask=0; itask<nthread; itask++) {
        
        struct task_arg *arg = malloc(sizeof(struct task_arg));
        arg->cpu = itask%ncpu;
        char ofilename[256] = {0};
        sprintf(ofilename, "%s.%d", ofile, itask);
        
        if(ofile) {
            arg->fp_ofile = fopen(nthreads==1?ofile:ofilename, "w");
        } else {
            arg->fp_ofile = stderr;
        }
        
        pthread_create(&threads[itask], NULL, task_routine, arg);
        sprintf(thread_name, "rtoax:%d", itask);
        pthread_setname_np(threads[itask], thread_name);
        memset(thread_name, 0, sizeof(thread_name));
    }
    
    for(itask=0; itask<nthread; itask++) {
        struct task_arg *arg = NULL;
        pthread_join(threads[itask], (void**)&arg);
        free(arg);
    }
    
    
}

