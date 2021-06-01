#include <stdio.h>
#include <unistd.h>

#include "ring.h"

struct msg {
#define MSG_MAGIC   0x1234abcd
    int magic;
    char pad[];
};

int main()
{
    struct __ring* ring = __ring_create(getpagesize());
    int i = 0;
    
    while(1) {
        i ++;
        size_t size = sizeof(struct msg) + i;
        struct msg *m = malloc(size);
        m->magic = MSG_MAGIC;
        if(!__ring_enqueue(ring, m, size)) {
            free(m);
            break;
        }
        free(m);
    }

    struct msg m2;
    size_t size;
    while(__ring_dequeue(ring, &m2, &size)) {
        printf("msg.magic = 0x%x, size = %ld\n", m2.magic, size);
//        sleep(1);
    }
}
