#include <stdio.h>

#include "rte_ticketlock.h"

//测量自旋锁造成的时延


rte_ticketlock_t spinlock;
volatile unsigned long ticks_start = 0;
volatile unsigned long ticks_end = 0;


static struct cpu_freq {
    enum {CPU_3GMHZ, CPU_2_7GMHZ,}index;
    unsigned long freq;
    char *string;
}CPU_MHZ[] = {
    {CPU_3GMHZ, 3000000000, "3GMHz"},
    {CPU_2_7GMHZ, 2700000000, "2.7GMHz"},
};

unsigned long get_x84_64_ticks(void)
{
    unsigned long ret;
    union {
        unsigned long tsc_64;
        struct {
            unsigned int lo_32;
            unsigned int hi_32;
        };
    } tsc;
    __asm volatile("rdtsc" :
             "=a" (tsc.lo_32),
             "=d" (tsc.hi_32));
     ret = ((unsigned long)tsc.tsc_64);
     return ret;
}


void* task_hold_spin(void* unused)
{
	printf("task_hold_spin.\n");
    rte_ticketlock_lock(&spinlock);
    
    sleep(1);
    ticks_start = get_x84_64_ticks();
    
    rte_ticketlock_unlock(&spinlock);
    pthread_exit(NULL);
	return NULL;
}

void* task_get_spin(void* unused)
{
	printf("task_get_spin.\n");

    sleep(1);
    rte_ticketlock_lock(&spinlock);
    ticks_end = get_x84_64_ticks();
    
    rte_ticketlock_unlock(&spinlock);

    pthread_exit(NULL);
	return NULL;
}


/* The main program. */
int main ()
{
	pthread_t t1, t2;

    rte_ticketlock_init(&spinlock);
    
	pthread_create(&t1, NULL, task_hold_spin, NULL);
	pthread_create(&t2, NULL, task_get_spin, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

//    pthread_spin_destroy(&spinlock);

    
    printf("Latency = %lf ms\n", (ticks_end-ticks_start)*1.0/CPU_MHZ[CPU_2_7GMHZ].freq*1000.0);
    
	return 0;
}






