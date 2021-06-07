#define _GNU_SOURCE             /* See feature_test_macros(7) */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "ring.h"

#include "../../fastq/utils.h"


struct msg {
#define MSG_MAGIC   0x1234abcd
    int magic;
    int size;
//    int slot;
};

struct __ring* ring = NULL;

//#define NR_SLOTS    10
//static unsigned long statistics_slot_enqueue[NR_SLOTS] = {0};
//static unsigned long statistics_slot_dequeue[NR_SLOTS] = {0};
//static unsigned long statistics_slot_dequeue_magic_error[NR_SLOTS] = {0};


void *enqueue(void*arg)
{
    reset_self_cpuset("1");
    
    struct msg *m = (struct msg *)arg;
    size_t size = sizeof(struct msg);
    
    int i = 0;
    
    while(1) {
        
        m->magic = MSG_MAGIC;

//        m->slot = i%NR_SLOTS;
        m->size = size;

//        statistics_slot_enqueue[m->slot]++;

        while(!__ring_enqueue(ring, m, size)) {
            __relax();
        }
//        sleep(1);
        i ++;
    }
}

void *dequeue(void*arg)
{
    reset_self_cpuset("0");
    
    struct timeval start, end;
    char buffer[1024];
    size_t size;
    unsigned long total_dequeue = 0;

    gettimeofday(&start, NULL);
    
    while(1) {
        while(__ring_dequeue(ring, buffer, &size)) {
            total_dequeue++;
            struct msg *m = (struct msg *)buffer;

//            if(unlikely(m->magic != MSG_MAGIC)) {
//                assert(0);
//                printf("error: size = %d, magic=0x%#0x\n", size, m->magic);
//                statistics_slot_dequeue_magic_error[m->slot]++;
//            }
//            statistics_slot_dequeue[m->slot]++;

            
            if(unlikely(total_dequeue % 10000000 == 0)) {
                
                gettimeofday(&end, NULL);

                unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
                double nmsg_per_sec = (double)((total_dequeue)*1.0 / usec) * 1000000;
                printf("\nTotal = %ld, %ld/sec\n", total_dequeue, (unsigned long )nmsg_per_sec);
                total_dequeue = 0;

                gettimeofday(&start, NULL);
//                int i;
//                printf("enqueue: ");
//                for(i=0; i<NR_SLOTS; i++) {
//                    printf("%-9ld ", statistics_slot_enqueue[i]);
//                }
//                printf("\n");
//                printf("dequeue: ");
//                for(i=0; i<NR_SLOTS; i++) {
//                    printf("%-9ld ", statistics_slot_dequeue[i]);
//                }
//                printf("\n");
//                printf("dequ[E]: ");
//                for(i=0; i<NR_SLOTS; i++) {
//                    printf("%-9ld ", statistics_slot_dequeue_magic_error[i]);
//                }
//                printf("\n");
            }
        }
        __relax();
    }
}


int main()
{
    unsigned int ring_size = getpagesize()*10;
//    unsigned int ring_size = 128;
    ring = __ring_create(ring_size);

    pthread_t tasks[4];

    pthread_create(&tasks[0], NULL, dequeue, NULL);

//    char *enqueue_buffer = malloc(1024);
    char enqueue_buffer[1024];
    pthread_create(&tasks[1], NULL, enqueue, enqueue_buffer);

    pthread_join(tasks[0], NULL);
    pthread_join(tasks[1], NULL);

    return 0;
}
