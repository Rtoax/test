enum {
    NODE_1 = 1, 
    NODE_2,
    NODE_3,
    NODE_4,
};

/* 测试的消息总数 */
#ifndef TEST_NUM
#define TEST_NUM   (1UL<<20)
#endif

typedef struct  {
#define TEST_MSG_MAGIC 0x123123ff    
    unsigned long value;
    int magic;
    uint64_t latency;
}__attribute__((aligned(64))) test_msgs_t;

struct enqueue_arg {
    int srcModuleId;
    char *cpu_list;
    test_msgs_t *msgs;
};

struct dequeue_arg {
    char *cpu_list;
};


static char *global_cpu_lists[10] = {
    "0",    //dequeue
    "1",    //enqueue1
    "2",    //enqueue2
    "3",    //enqueue3
    "4",    //enqueue4
    "5",    //enqueue5
    "6",    
    "7",    
    "8",    
    "9",    
};

