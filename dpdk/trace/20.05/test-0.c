/**
 *  DPDK 20.05 trace
 *
 *  格式 CTF, 参见 LTTng
 *  
 *  
 *  二进制结果解析方式：
 *  babeltrace /home/rongtao/dpdk-traces/rte-2021-04-15-PM-06-11-04
 */

#include <pthread.h>

#include <rte_common.h>
#include <rte_errno.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_string_fns.h>
#include <rte_eal_trace.h>

#include "eal_trace.h"

#define NR_TASKS 2

void rte_eal_thread_remote_launch(int (*cb)(void*), void *arg, int lcore_id) {
    cb(arg);
	rte_eal_trace_thread_remote_launch(cb, arg, lcore_id, 1);

}
int cb1(void*arg) {}

void* my_malloc(size_t size) {
    rte_eal_trace_mem_malloc("RT", size, 64, 0, NULL);
    return malloc(size);
}
void my_free(void* ptr) {
    rte_eal_trace_mem_free(ptr);
    return free(ptr);
}

void *task_routine(void *arg) {
    int i;
    for(i=0 ; i< 4; i++) {
        char *s1 = my_malloc(256);
        rte_eal_thread_remote_launch(cb1, s1, 1);
        my_free(s1);
    }
    pthread_exit(NULL);
}

int main() {

    int i;

    pthread_t tasks[NR_TASKS];

    eal_trace_init();

    rte_trace_point_enable(&__rte_eal_trace_thread_remote_launch);
    rte_trace_point_enable(&__rte_eal_trace_mem_malloc);
    rte_trace_point_enable(&__rte_eal_trace_mem_free);
    rte_trace_point_disable(&__rte_eal_trace_generic_void);

    for(i=0 ; i< NR_TASKS; i++) {
        pthread_create(&tasks[i], NULL, task_routine, NULL);
    }

    for(i=0 ; i< NR_TASKS; i++) {
       pthread_join(tasks[i], NULL);
   }

    

    rte_trace_dump(stdout);	

    rte_trace_save();
	eal_trace_fini();
}
