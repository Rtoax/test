#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "staging-buffer.h"


struct test_msg {
#define MAGIC_NUMBER    0x12341234
    int magic;
    char pad[3];
};

struct StagingBuffer staging_buf;


void *enqueue(void*arg)
{
    while(1) {
        char *buf = reserveProducerSpace(&staging_buf, sizeof(struct test_msg));
        struct test_msg *msg = (struct test_msg*)buf;

        msg->magic = MAGIC_NUMBER;

        finishReservation(&staging_buf, sizeof(struct test_msg));
//        sleep(1);
    }
}

void *dequeue(void*arg)
{
    size_t size = 8;
    struct test_msg *msg = NULL;

    while(1) {
//        sleep(1);
        
        msg = (struct test_msg*)peek(&staging_buf, &size);

        if(!msg) {
            printf("peek error.\n");
            continue;
        }

        printf("magic = 0x%0#x, size = %d(%ld)\n", msg->magic, size, staging_buf.producerPos - staging_buf.storage);
        
        consume(&staging_buf, size);
    }
}


int main()
{
    size_t i;
    pthread_t tasks[4];

    staging_buf.endOfRecordedSpace = staging_buf.storage + STAGING_BUFFER_SIZE;
    staging_buf.minFreeSpace = 0;

    staging_buf.cyclesProducerBlocked = (0);
    staging_buf.numTimesProducerBlocked = (0);
    staging_buf.numAllocations = (0);

    staging_buf.cyclesIn10Ns = 1000;
    staging_buf.consumerPos = staging_buf.storage;
    staging_buf.shouldDeallocate = false;
    staging_buf.id = 1;//bufferId;
    staging_buf.producerPos = staging_buf.storage;

    
    for (i = 0; i < 20; ++i)
    {
        staging_buf.cyclesProducerBlockedDist[i] = 0;
    }

    
    pthread_create(&tasks[0], NULL, dequeue, NULL);
    pthread_create(&tasks[1], NULL, enqueue, NULL);

    pthread_join(tasks[0], NULL);
    pthread_join(tasks[1], NULL);

    return 0;
}

