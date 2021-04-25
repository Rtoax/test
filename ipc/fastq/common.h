enum {
    NODE_1 = 1, 
    NODE_2,
    NODE_3,
    NODE_4,
    NODE_5,
    NODE_6,
    NODE_7,
    NODE_8,
    NODE_9,
    NODE_10,
    NODE_11,
    NODE_12,
    NODE_13,
    NODE_14,
    NODE_15,
    NODE_16,
    NODE_17,
    NODE_NUM
};

const char *ModuleName[] = {
#define __(a) [a] = #a
__(NODE_1),
__(NODE_2),
__(NODE_3),
__(NODE_4),
__(NODE_5),
__(NODE_6),
__(NODE_7),
__(NODE_8),
__(NODE_9),
__(NODE_10),
__(NODE_11),
__(NODE_12),
__(NODE_13),
__(NODE_14),
__(NODE_15),
__(NODE_16),
__(NODE_17),
#undef __
};

/* 测试的消息总数 */
#ifndef TEST_NUM
#define TEST_NUM   (1UL<<20)
#endif

typedef struct  {
#define TEST_MSG_MAGIC 0x123123ff    
    unsigned long msgType;
    unsigned long msgCode;
    unsigned long value;
    int magic;
    uint64_t latency;
}__attribute__((aligned(64))) test_msgs_t;

struct enqueue_arg {
    int srcModuleId;
    int dstModuleId;
    char *cpu_list;
    test_msgs_t *msgs;
};

struct dequeue_arg {
    int srcModuleId;
    int dstModuleId;
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


static bool moduleID_filter_fn(unsigned long srcID, unsigned long dstID){
//    if(srcID != NODE_1 && 
//       srcID != NODE_2 && 
//       srcID != NODE_3 && 
//       srcID != NODE_4 && 
//       srcID != 0) return false;
//    
//    if(dstID == NODE_1) return true;
//    else return false;
//    printf("filter.\n");
    return true;
}

static dump_all_fastq() {

    FastQDump(stderr, 0);

    struct FastQModuleMsgStatInfo buffer[32];
    unsigned int num = 0;
    bool ret = VOS_FastQMsgStatInfo(buffer, 32, &num, moduleID_filter_fn);
//        sleep(1);
//        printf("statistics. ret = %d, %d\n", ret, num);

    if(num) {
        printf( "\t SRC -> DST           Enqueue           Dequeue\r\n");
        printf( "\t ----------------------------------------------------\r\n");
    }
    int i;
    for(i=0;i<num;i++) {
        
        printf( "\t %3ld -> %3ld:  %16ld %16ld\r\n", 
                buffer[i].src_module, buffer[i].dst_module, buffer[i].enqueue, buffer[i].dequeue);
    }
}
