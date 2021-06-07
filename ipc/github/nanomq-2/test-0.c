#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>

#include <nanomq.h>


#ifndef TEST_NUM
#define TEST_NUM   (1UL<<20)
#endif

#define MODULE_1 1
#define MODULE_2 2


/**
 *  获取tick
 */
#ifndef RDTSC
#define RDTSC() ({\
    register uint32_t a,d; \
    __asm__ __volatile__( "rdtsc" : "=a"(a), "=d"(d)); \
    (((uint64_t)a)+(((uint64_t)d)<<32)); \
    })
#endif

typedef struct  {
#define TEST_MSG_MAGIC 0x123123ff    
    int magic;
    unsigned long value;
    uint64_t latency;
}__attribute__((aligned(64))) test_msgs_t;


test_msgs_t *test_msgs;
uint64_t latency_total = 0;
uint64_t total_msgs = 0;
uint64_t error_msgs = 0;
struct nmq_context ctx1;

void *enqueue_task(void*arg){
    struct nmq_context *ctx = (struct nmq_context *)arg;
    int i =0;
    test_msgs_t *pmsg;
    while(1) {
        pmsg = &test_msgs[i++%TEST_NUM];
        pmsg->latency = RDTSC();
        unsigned long addr = (unsigned long)pmsg;
        nmq_ctx_sendto(ctx, MODULE_1, MODULE_2, &addr, sizeof(unsigned long));
    }
    pthread_exit(NULL);
}

void *dequeue_task(void*arg){
    struct nmq_context *ctx = (struct nmq_context *)arg;

    size_t sz = sizeof(unsigned long);
    test_msgs_t *pmsg;
    unsigned long addr;

    struct timeval start, end;
    
    gettimeofday(&start, NULL);
    while(1) {
//        usleep(1000);
        nmq_ctx_recvfrom(ctx, MODULE_1, MODULE_2, &addr, &sz);
        pmsg = (test_msgs_t *)addr;
    
        latency_total += RDTSC() - pmsg->latency;
        pmsg->latency = 0;
        if(pmsg->magic != TEST_MSG_MAGIC) {
            error_msgs++;
        }
        
        total_msgs++;

        if(total_msgs % 5000000 == 0) {
            printf("dequeue. per msgs \033[1;31m%lf ns\033[m, msgs (total %ld, err %ld).\n", 
                    latency_total*1.0/total_msgs/3000000000*1000000000,
                    total_msgs, error_msgs);
            gettimeofday(&end, NULL);

            unsigned long usec = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
            double nmsg_per_sec = (double)((total_msgs)*1.0 / usec) * 1000000;
            printf("Total = %ld, %ld/sec\n\n", total_msgs, (unsigned long )nmsg_per_sec);

            total_msgs = 0;
            latency_total = 0;

            gettimeofday(&start, NULL);
        }
    }
    pthread_exit(NULL);
}



int sig_handler(int signum) {

    nmq_ctx_print(&ctx1);

    exit(1);
}

int main()
{
    pthread_t enqueue_taskid, dequeue_taskid;

    
    signal(SIGINT, sig_handler);
    
    nmq_ctx_create(&ctx1, 4, 8, 1024);
    unsigned int i =0;
    test_msgs = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
    for(i=0;i<TEST_NUM;i++) {
//        test_msgs[i].magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有错误的消息
        test_msgs[i].magic = TEST_MSG_MAGIC; //有错误的消息
        test_msgs[i].value = i+1;
    }


    pthread_create(&enqueue_taskid, NULL, enqueue_task, &ctx1);
    pthread_create(&dequeue_taskid, NULL, dequeue_task, &ctx1);

    pthread_join(enqueue_taskid, NULL);
    pthread_join(dequeue_taskid, NULL);

    return EXIT_SUCCESS;
}



