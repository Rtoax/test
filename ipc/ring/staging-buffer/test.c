#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#include "staging-buffer.h"


struct test_msg {
#define MAGIC_NUMBER    0x12341234
    int magic;
    int size;
    char pad[1];
};

struct StagingBuffer *staging_buf = NULL;


void *enqueue(void*arg)
{
    int i = 0;
    while(1) {
        size_t size = sizeof(struct test_msg) + i%20;
        char *buf = reserveProducerSpace(staging_buf, size);
        assert(buf);
        struct test_msg *msg = (struct test_msg*)buf;

        msg->magic = MAGIC_NUMBER;
        msg->size = size;

        finishReservation(staging_buf, size);
//        usleep(20000);
        i++;
    }
}

void *dequeue(void*arg)
{
    size_t size, remain;
    struct test_msg *msg = NULL;
    unsigned long int N_consume = 0;
    struct timeval start, end;
    usleep(10000);
    gettimeofday(&start, NULL);
    size_t __size = 0;

    while(1) {
        
        msg = (struct test_msg*)peek_buffer(staging_buf, &size);

        if(!msg) {
            printf("peek error.\n");
            continue;
        }
        
        remain = size;
        while(remain > 0) {
            __size = msg->size;
//            printf("magic = 0x%0#x, size = %d, remain=%d\n", msg->magic, __size, remain);
            char *p = (char *)msg;
            N_consume ++;
            p += __size;
            remain -= __size;
            msg = (struct test_msg*)p;
        }
        
        consume_done(staging_buf, size);
        
//        usleep(20000);
        
//        N_consume ++;
        if((N_consume > 10000000)) {
            
            gettimeofday(&end, NULL);

            unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
            double nmsg_per_sec = (double)((N_consume)*1.0 / usec) * 1000000;
            printf("\nTotal = %ld, %ld/sec\n", N_consume, (unsigned long )nmsg_per_sec);
            N_consume = 0;

            gettimeofday(&start, NULL);
        }
    }
}


int main()
{
    size_t i;
    pthread_t tasks[4];
    staging_buf = create_buff();

    
    pthread_create(&tasks[0], NULL, dequeue, NULL);
    pthread_create(&tasks[1], NULL, enqueue, NULL);

    pthread_join(tasks[0], NULL);
    pthread_join(tasks[1], NULL);

    return 0;
}

