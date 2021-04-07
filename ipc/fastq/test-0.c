/**********************************************************************************************************************\
*  文件： test-0.c
*  介绍： 低时延队列 多入单出队列 调试功能测试 测试例
*  作者： 荣涛
*  日期：
*       2021年2月2日    
\**********************************************************************************************************************/
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>

#define _FASTQ_STATS
#include <fastq.h>

#include "common.h"


void sig_handler(int signum) {

    FastQDump(NULL, NODE_1);
    exit(1);
}

bool moduleID_filter_fn(unsigned long srcID, unsigned long dstID){
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


int main()
{
    int max_msg = 16;
    
    signal(SIGINT, sig_handler);

    mod_set rxset, txset;

    MOD_ZERO(&rxset);
    MOD_ZERO(&txset);

    MOD_SET(NODE_1, &rxset);
    MOD_SET(NODE_2, &rxset);
//    MOD_SET(NODE_3, &rxset);
//    MOD_SET(NODE_4, &rxset);
//    MOD_SET(NODE_1, &txset);
//    MOD_SET(NODE_2, &txset);
    MOD_SET(NODE_3, &txset);
    MOD_SET(NODE_4, &txset);
    
    
    FastQCreateModuleStats(NODE_1, &rxset, &txset, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModuleStats(NODE_2, &rxset, &txset, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModuleStats(NODE_3, &rxset, &txset, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModuleStats(NODE_4, &rxset, &txset, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    

    FastQDump(stderr, 0);

    while(1) {
        struct FastQModuleMsgStatInfo buffer[32];
        unsigned int num = 0;
        bool ret = FastQMsgStatInfoStats(buffer, 32, &num, moduleID_filter_fn);
        sleep(1);
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

    return EXIT_SUCCESS;
}




