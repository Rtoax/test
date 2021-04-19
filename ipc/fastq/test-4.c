/**********************************************************************************************************************\
*  文件： test-4.c
*  介绍： 低时延队列 多入单出队列 通知+轮询接口测试例
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

//#define _FASTQ_STATS //开启统计功能
#include <fastq.h>

#include "common.h"

uint64_t latency_total = 0;
uint64_t total_msgs = 0;
uint64_t error_msgs = 0;


void *enqueue_task(void*arg){
    int i =0;
    struct enqueue_arg *parg = (struct enqueue_arg *)arg;
    reset_self_cpuset(parg->cpu_list);
    test_msgs_t *ptest_msg = parg->msgs;
    test_msgs_t *pmsg;
    
    unsigned long send_cnt = 0;
    
    while(1) {
//        usleep(1000000);
        pmsg = &ptest_msg[i++%TEST_NUM];
        pmsg->latency = RDTSC();
//        printf("send %lx(%lx)\n", pmsg->value, *(unsigned long*)pmsg);
        VOS_FastQTrySend(parg->srcModuleId, NODE_1, pmsg, sizeof(test_msgs_t));

        send_cnt++;
        
        if(send_cnt % 2000000 == 0) {
            sleep((unsigned long)(arg)%10);
        }
    }
    pthread_exit(NULL);
}

void handler_test_msg(void* msg, size_t size)
{
    test_msgs_t *pmsg = (test_msgs_t *)msg;
    unsigned long nr_en, nr_de, nr_curr;
    
//    printf("recv %lx\n", pmsg->value);
    
    latency_total += RDTSC() - pmsg->latency;
    pmsg->latency = 0;
    if(pmsg->magic != TEST_MSG_MAGIC) {
        error_msgs++;
    }
    
    total_msgs++;

    if(total_msgs % 2000000 == 0) {
        VOS_FastQMsgNum(NODE_1, &nr_en, &nr_de, &nr_curr);
        printf("dequeue. per msgs \033[1;31m%lf ns\033[m, msgs (total %ld, err %ld). "\
               "EN %ld, DE %ld, CURR %ld\n", 
               latency_total*1.0/total_msgs/3000000000*1000000000,
               total_msgs, error_msgs, nr_en, nr_de, nr_curr);
    }

}

void *dequeue_task(void*arg) {
    struct dequeue_arg *parg = (struct dequeue_arg*)arg;
    reset_self_cpuset(parg->cpu_list);
    
    VOS_FastQRecv( NODE_1, handler_test_msg);
    pthread_exit(NULL);
}

int sig_handler(int signum) {
    printf("sighandler.\n");
    VOS_FastQDump(NULL, NODE_1);
    
    exit(1);
}

pthread_t new_enqueue_task(int module_id) {
    pthread_t enqueue;
    int max_msg = 16;
    unsigned int i =0;
    struct enqueue_arg *enqueueArg = malloc(sizeof(struct enqueue_arg));

    mod_set rxset, txset;
    MOD_ZERO(&rxset);
    MOD_ZERO(&txset);
    
    MOD_SET(NODE_1, &txset);
    VOS_FastQCreateModule(ModuleName[module_id], module_id, NULL, &txset, max_msg, sizeof(test_msgs_t));

    
    MOD_SET(module_id, &rxset);
    VOS_FastQAddSet(NODE_1, &rxset, NULL);

    
    test_msgs_t *test_msg = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
    for(i=0;i<TEST_NUM;i++) {
        test_msg->magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有错误的消息
//        test_msgs21[i].magic = TEST_MSG_MAGIC;
        test_msg->value = 0xff00000000000000 + i+1;
    }


    enqueueArg->srcModuleId = module_id;
    enqueueArg->cpu_list = global_cpu_lists[module_id-1-2];
    enqueueArg->msgs = test_msg;

    sleep(1);

    pthread_create(&enqueue, NULL, enqueue_task, enqueueArg);

    return enqueue;
}

int main()
{
    pthread_t enqueueTask1;
    pthread_t enqueueTask2;
    pthread_t enqueueTask3;
    pthread_t dequeueTask;
    
    test_msgs_t *test_msgs21;
    test_msgs_t *test_msgs31;
    test_msgs_t *test_msgs41;
    
    int max_msg = 16;
    
    signal(SIGINT, sig_handler);
    
    mod_set rxset, txset;
    MOD_ZERO(&rxset);
    MOD_ZERO(&txset);

    MOD_SET(NODE_2, &rxset);
    MOD_SET(NODE_3, &rxset);
    MOD_SET(NODE_4, &rxset);
    
    MOD_SET(NODE_1, &txset);
    VOS_FastQCreateModule(ModuleName[NODE_1], NODE_1, &rxset, NULL, max_msg, sizeof(test_msgs_t));
    VOS_FastQCreateModule(ModuleName[NODE_2], NODE_2, NULL, &txset, max_msg, sizeof(test_msgs_t));
    VOS_FastQCreateModule(ModuleName[NODE_3], NODE_3, NULL, &txset, max_msg, sizeof(test_msgs_t));
    VOS_FastQCreateModule(ModuleName[NODE_4], NODE_4, NULL, &txset, max_msg, sizeof(test_msgs_t));
    
    unsigned int i =0;
    test_msgs21 = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
    for(i=0;i<TEST_NUM;i++) {
        test_msgs21[i].magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有错误的消息
//        test_msgs21[i].magic = TEST_MSG_MAGIC;
        test_msgs21[i].value = 0xff00000000000000 + i+1;
    }
    test_msgs31 = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
    for(i=0;i<TEST_NUM;i++) {
        test_msgs31[i].magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有错误的消息
//        test_msgs21[i].magic = TEST_MSG_MAGIC;
        test_msgs31[i].value = 0xffff000000000000 + i+1;
    }
    test_msgs41 = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
    for(i=0;i<TEST_NUM;i++) {
        test_msgs41[i].magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有错误的消息
//        test_msgs21[i].magic = TEST_MSG_MAGIC;
        test_msgs41[i].value = 0xffffff0000000000 + i+1;
    }
    struct dequeue_arg dequeueArg;
    struct enqueue_arg enqueueArg1;
    struct enqueue_arg enqueueArg2;
    struct enqueue_arg enqueueArg3;

    dequeueArg.cpu_list = global_cpu_lists[NODE_1-1];

    enqueueArg1.srcModuleId = NODE_2;
    enqueueArg1.cpu_list = global_cpu_lists[NODE_2-1];
    enqueueArg1.msgs = test_msgs21;
    enqueueArg2.srcModuleId = NODE_3;
    enqueueArg2.cpu_list = global_cpu_lists[NODE_3-1];
    enqueueArg2.msgs = test_msgs31;
    enqueueArg3.srcModuleId = NODE_4;
    enqueueArg3.cpu_list = global_cpu_lists[NODE_4-1];
    enqueueArg3.msgs = test_msgs41;
    

    pthread_create(&enqueueTask1, NULL, enqueue_task, &enqueueArg1);
    pthread_create(&enqueueTask2, NULL, enqueue_task, &enqueueArg2);
    pthread_create(&enqueueTask3, NULL, enqueue_task, &enqueueArg3);
    pthread_create(&dequeueTask, NULL, dequeue_task, &dequeueArg);

	pthread_setname_np(enqueueTask1, "enqueue1");
	pthread_setname_np(enqueueTask2, "enqueue2");
	pthread_setname_np(enqueueTask3, "enqueue3");
	pthread_setname_np(dequeueTask,  "dequeue");

	pthread_setname_np(pthread_self(), "test-1");

    sleep(5);
    pthread_t enqueue_add1 = new_enqueue_task(NODE_5);
	pthread_setname_np(enqueue_add1, "enqueue4");
    sleep(5);
    pthread_t enqueue_add2 = new_enqueue_task(NODE_6);
	pthread_setname_np(enqueue_add2, "enqueue5");

    pthread_join(enqueueTask1, NULL);
    pthread_join(enqueueTask2, NULL);
    pthread_join(enqueueTask3, NULL);
    pthread_join(dequeueTask, NULL);
    pthread_join(enqueue_add1, NULL);
    pthread_join(enqueue_add2, NULL);

    return EXIT_SUCCESS;
}





