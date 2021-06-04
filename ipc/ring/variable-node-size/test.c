#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "ring.h"

struct msg {
#define MSG_MAGIC   0x1234abcd
    int magic;
    int size;
    int magic2;
    int magic3;
    int magic4;
    int magic5;
    int slot;
    int check1;
    int check2;
    char pad[];
};

struct __ring* ring = NULL;

#define NR_SLOTS    10
static volatile unsigned long statistics_slot_enqueue[NR_SLOTS] = {0};
static volatile unsigned long statistics_slot_dequeue[NR_SLOTS] = {0};
static volatile unsigned long statistics_slot_dequeue_magic_error[NR_SLOTS] = {0};


void *enqueue(void*arg)
{
    int i = 0;
    while(1) {
        size_t size = sizeof(struct msg) + i%100+sizeof(int);
        struct msg *m = (struct msg *)malloc(size);
        if(!m) {
            printf("malloc error.\n");
            sleep(1);
            continue;
        }
        m->magic = m->magic2 = m->magic3 = m->magic4 = m->magic5 = MSG_MAGIC;
        m->slot = i%NR_SLOTS;
        m->size = size;

        /* 最后4个字节设置为 magic  */
        *(int*)(((char*)m) + size - sizeof(int)) = MSG_MAGIC;
        
        statistics_slot_enqueue[m->slot]++;
        
        while(!__ring_enqueue(ring, m, size)) {
            __relax();
        }
//        sleep(1);
        free(m);
        i ++;
    }
}

void *dequeue(void*arg)
{
    struct timeval start, end;
    char buffer[1024];
    size_t size;
    unsigned long total_dequeue = 0;

    gettimeofday(&start, NULL);
    
    while(1) {
        while(__ring_dequeue(ring, buffer, &size)) {
            total_dequeue++;
            struct msg *m = (struct msg *)buffer;

            /* 最后4个字节设置为 magic  */
            int *_last_magic = (int*)(((char*)m)+size-sizeof(int));
            
            if(m->magic != MSG_MAGIC || 
               m->magic2 != MSG_MAGIC || 
               m->magic3 != MSG_MAGIC || 
               m->magic4 != MSG_MAGIC || 
               m->magic5 != MSG_MAGIC ||
               *_last_magic != MSG_MAGIC) {
                printf("error: size = %d(%d), slot=%d, magic=0x%#0x\n", size, m->size, m->slot, m->magic);
                statistics_slot_dequeue_magic_error[m->slot]++;
            }
            statistics_slot_dequeue[m->slot]++;

            
            
            if(total_dequeue % 1000000 == 0) {
                static unsigned long last_total_dequeue = 0;
                
                gettimeofday(&end, NULL);

                unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
                double nmsg_per_sec = (double)((total_dequeue - last_total_dequeue)*1.0 / usec) * 1000000;
                printf("\nTotal = %ld, %ld/sec\n", total_dequeue, (unsigned long )nmsg_per_sec);
                last_total_dequeue = total_dequeue;

                gettimeofday(&start, NULL);
                
                int i;
                printf("enqueue: ");
                for(i=0; i<NR_SLOTS; i++) {
                    printf("%-9ld ", statistics_slot_enqueue[i]);
                }
                printf("\n");
                printf("dequeue: ");
                for(i=0; i<NR_SLOTS; i++) {
                    printf("%-9ld ", statistics_slot_dequeue[i]);
                }
                printf("\n");
                printf("dequ[E]: ");
                for(i=0; i<NR_SLOTS; i++) {
                    printf("%-9ld ", statistics_slot_dequeue_magic_error[i]);
                }
                printf("\n");
            }
        }
        __relax();
    }
}


int main()
{
    unsigned int ring_size = getpagesize()*100;
//    unsigned int ring_size = 128;
    ring = __ring_create(ring_size);

    pthread_t tasks[4];

    pthread_create(&tasks[0], NULL, dequeue, NULL);
    pthread_create(&tasks[1], NULL, enqueue, NULL);

    pthread_join(tasks[0], NULL);
    pthread_join(tasks[1], NULL);

    return 0;
}
