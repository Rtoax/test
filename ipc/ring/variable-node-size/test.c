#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "ring.h"

struct msg {
#define MSG_MAGIC   0x1234abcd
    int magic;
    int magic2;
    int magic3;
    int magic4;
    int magic5;
    char pad[];
};

struct __ring* ring = NULL;


void *enqueue(void*arg)
{
    int i = 0;
    while(1) {
        i ++;
        size_t size = sizeof(struct msg) + i%100;
        struct msg *m = (struct msg *)malloc(size);
        if(!m) {
            printf("malloc error.\n");
            sleep(1);
            continue;
        }
        m->magic = MSG_MAGIC;
        while(!__ring_enqueue(ring, m, size)) {
            __relax();
        }
//        printf("enqueue: msg.magic = 0x%x, size = %ld\n", m->magic, size);
        free(m);
    }
}

void *dequeue(void*arg)
{
    char buffer[1024];
    size_t size;
    while(1) {
        while(__ring_dequeue(ring, buffer, &size)) {
            struct msg *m = (struct msg *)buffer;
//            printf("dequeue: msg.magic = 0x%x, size = %ld\n", m->magic, size);
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
