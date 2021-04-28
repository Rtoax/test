/**********************************************************************************************************************\
*  文件： test-3.c
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
#include <sys/time.h>
#include <pthread.h>

#include <fastq.h>

#include "common.h"

#define NR_PROCESSOR sysconf(_SC_NPROCESSORS_ONLN)

enum {
    MSGCODE_0,
    MSGCODE_1,
    MSGCODE_2,
    MSGCODE_3,
    MSGCODE_4,
    MSGCODE_MAX,
};

uint64_t latency_total = 0;
uint64_t total_msgs = 0;
uint64_t error_msgs = 0;
uint64_t send_failed = 0;
uint64_t msg_type_statistic[MSGCODE_MAX] = {0};
uint64_t msg_code_statistic[MSGCODE_MAX] = {0};
uint64_t msg_subcode_statistic[MSGCODE_MAX] = {0};
uint64_t msg_src_statistic[NODE_NUM] = {0};


test_msgs_t *test_msgs[NODE_NUM] = {NULL};


void *enqueue_task(void*arg){
    int i =0;
    bool ret;
    struct enqueue_arg *parg = (struct enqueue_arg *)arg;
    reset_self_cpuset(parg->cpu_list);
    test_msgs_t *ptest_msg = parg->msgs;
    test_msgs_t *pmsg;
    
    unsigned long send_cnt = 0;
    unsigned long srcModuleID = parg->srcModuleId;
    unsigned long dstModuleId = parg->dstModuleId;
    
    while(1) {
        pmsg = &ptest_msg[i%TEST_NUM];
        pmsg->msgType = i%MSGCODE_MAX;
        pmsg->msgCode = i%MSGCODE_MAX;
        pmsg->msgSubCode = i%MSGCODE_MAX;
        pmsg->latency = RDTSC();
        unsigned long addr = (unsigned long)pmsg;
        unsigned long msgType = pmsg->msgType;
        unsigned long msgCode = pmsg->msgCode;
        unsigned long msgSubCode = pmsg->msgSubCode;
        switch(i%4) {
            case 0:
                pmsg->msgCode = MSGCODE_1;
                pmsg->msgSubCode = MSGCODE_1;
                ret = VOS_FastQSend(srcModuleID, dstModuleId, msgType, msgCode, msgSubCode, &addr, sizeof(unsigned long));
                break;
            case 1:
                pmsg->msgCode = MSGCODE_2;
                pmsg->msgSubCode = MSGCODE_2;
                ret = VOS_FastQTrySend(srcModuleID, dstModuleId, msgType, msgCode, msgSubCode, &addr, sizeof(unsigned long));
                break;
            case 2:
                pmsg->msgCode = MSGCODE_3;
                pmsg->msgSubCode = MSGCODE_3;
                ret = VOS_FastQSendByName(ModuleName[srcModuleID], ModuleName[dstModuleId], msgType, msgCode, msgSubCode, &addr, sizeof(unsigned long));
                break;
            case 3:
                pmsg->msgCode = MSGCODE_4;
                pmsg->msgSubCode = MSGCODE_4;
                ret = VOS_FastQTrySendByName(ModuleName[srcModuleID], ModuleName[dstModuleId], msgType, msgCode, msgSubCode, &addr, sizeof(unsigned long));
                break;
        }
        i = ret?(i+1):i;
        if(!ret) send_failed++;

        if(ret) {
//            printf("send: %x,(%x) , ret = %d\n", pmsg->magic, TEST_MSG_MAGIC, ret);
//            sleep(1);
            send_cnt++;
        }
        
        if(send_cnt % 5000000 == 0) {
            printf("enqueue sleep(). send total %ld, failed %ld\n", send_cnt, send_failed);
            sleep(pmsg->latency%5);
        }
    }
    assert(0);
    pthread_exit(NULL);
}


void handler_test_msg(unsigned long src, unsigned long dst,unsigned long type, unsigned long code, unsigned long subcode, void* msg, size_t size)
{
    unsigned long addr =  *(unsigned long*)msg;
    test_msgs_t *pmsg;

    pmsg = (test_msgs_t *)addr;
    latency_total += RDTSC() - pmsg->latency;
    pmsg->latency = 0;
    if(pmsg->magic != TEST_MSG_MAGIC) {
        error_msgs++;
    }

    if(dst != NODE_1) {
        assert(0 && "Wrong dst moduleID.");
    }
//    printf("recv: %x,(%x) %ld\n", pmsg->magic, TEST_MSG_MAGIC, total_msgs);
    total_msgs++;
    msg_type_statistic[type]++;
    msg_code_statistic[code]++;
    msg_subcode_statistic[subcode]++;
    msg_src_statistic[src]++;
    
    if(total_msgs % 400000 == 0) {
        printf("dequeue. per msgs \033[1;31m%lf ns\033[m, msgs (total %ld,  err %ld).\n"\
                "                               Type[%8ld,%8ld,%8ld,%8ld,%8ld]\n"\
                "                               Code[%8ld,%8ld,%8ld,%8ld,%8ld]\n"\
                "                            SubCode[%8ld,%8ld,%8ld,%8ld,%8ld]\n"\
                "                                Src[%8ld,%8ld,%8ld,%8ld,%8ld]\n", 
                latency_total*1.0/400000/3000000000*1000000000,
                total_msgs,
                error_msgs, 
                msg_type_statistic[MSGCODE_0],
                msg_type_statistic[MSGCODE_1],
                msg_type_statistic[MSGCODE_2],
                msg_type_statistic[MSGCODE_3],
                msg_type_statistic[MSGCODE_4],
                
                msg_code_statistic[MSGCODE_0],
                msg_code_statistic[MSGCODE_1],
                msg_code_statistic[MSGCODE_2],
                msg_code_statistic[MSGCODE_3],
                msg_code_statistic[MSGCODE_4],
                
                msg_subcode_statistic[MSGCODE_0],
                msg_subcode_statistic[MSGCODE_1],
                msg_subcode_statistic[MSGCODE_2],
                msg_subcode_statistic[MSGCODE_3],
                msg_subcode_statistic[MSGCODE_4],
                
                msg_src_statistic[NODE_1],
                msg_src_statistic[NODE_2],
                msg_src_statistic[NODE_3],
                msg_src_statistic[NODE_4],
                msg_src_statistic[NODE_5]);
        latency_total = 0;
    }

}

void *dequeue_task(void*arg) {
    struct dequeue_arg *parg = (struct dequeue_arg*)arg;
    reset_self_cpuset(parg->cpu_list);
    
    VOS_FastQRecvByName( ModuleName[parg->srcModuleId], handler_test_msg);
    printf("Dequeue task exit.\n");
    pthread_exit(NULL);
}


pthread_t new_enqueue_task(unsigned long moduleId, unsigned long dstModuleId, char *moduleName, 
                               unsigned long *RxArray, int nRx,
                               unsigned long *TxArray, int nTx,
                               size_t maxMsg, size_t msgSize,
                               const char *cpulist, bool new_task) {
    pthread_t task = 0;
    unsigned int i =0;

    mod_set rxset, txset;
    MOD_ZERO(&rxset);
    MOD_ZERO(&txset);
    for(i=0; i<nRx; i++) {
        MOD_SET(RxArray[i], &rxset);
    }
    for(i=0; i<nTx; i++) {
        MOD_SET(TxArray[i], &txset);
    }
    
    VOS_FastQCreateModule(moduleId, &rxset, &txset, maxMsg, msgSize);
    if(moduleName)
        VOS_FastQAttachName(moduleId, ModuleName[moduleId]);

    test_msgs_t *test_msg = NULL;

    if(!test_msgs[moduleId]) {
        test_msgs[moduleId] = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
        test_msg = test_msgs[moduleId];
        for(i=0;i<TEST_NUM;i++) {
//            test_msg[i].magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有比特错误的消息
            test_msg[i].magic = TEST_MSG_MAGIC; 
            test_msg[i].value = 0xff00000000000000 + i+1;
        }
    } else {
        test_msg = test_msgs[moduleId];
    }

    if(new_task) {
        struct enqueue_arg *enqueueArg = malloc(sizeof(struct enqueue_arg));

        enqueueArg->srcModuleId = moduleId;
        enqueueArg->dstModuleId = dstModuleId;
        enqueueArg->cpu_list = strdup(cpulist);
        enqueueArg->msgs = test_msg;

        pthread_create(&task, NULL, enqueue_task, enqueueArg);
    	pthread_setname_np(task, moduleName?moduleName:"enqueue");
    }

    return task;
}



static pthread_t new_dequeue_task(unsigned long moduleId, char *moduleName, 
                               unsigned long *RxArray, int nRx,
                               unsigned long *TxArray, int nTx,
                               size_t maxMsg, size_t msgSize,
                               const char *cpulist) {
    pthread_t task;
    int i;
    struct dequeue_arg *taskArg = NULL;
    
    mod_set rxset, txset;
    MOD_ZERO(&rxset);
    MOD_ZERO(&txset);
    
    for(i=0; i<nRx; i++) {
        MOD_SET(RxArray[i], &rxset);
    }
    for(i=0; i<nTx; i++) {
        MOD_SET(TxArray[i], &txset);
    }

    VOS_FastQCreateModule(moduleId, NULL, &txset, maxMsg, msgSize);

    if(moduleName)
        VOS_FastQAttachName(moduleId, ModuleName[moduleId]);

    taskArg = malloc(sizeof(struct dequeue_arg));

    taskArg->cpu_list = strdup(cpulist);
    taskArg->srcModuleId = moduleId;
    
    pthread_create(&task, NULL, dequeue_task, taskArg);
	pthread_setname_np(task, moduleName?moduleName:"dequeue");

    return task;
}


int sig_handler(int signum) {

   switch(signum) {
       case SIGINT:
           VOS_FastQDump(NULL, NODE_1);
           exit(1);
           break;
       case SIGALRM:
           dump_all_fastq();
           break;
   }
}

pthread_t start_enqueue(int node, size_t msgNum, bool new_task) {
    unsigned long TXarr[] = {NODE_1};
    return new_enqueue_task(node, NODE_1, ModuleName[node], 
                                    NULL, 0, TXarr, 0, 
                                    msgNum, sizeof(long), 
                                    global_cpu_lists[(node-1)%NR_PROCESSOR], new_task);

}

int main()
{
    pthread_t dequeueTask;
    pthread_t enqueueTask;
    unsigned long i;
    size_t msgNum = 64;
    struct itimerval sa;
    sa.it_value.tv_sec = 100;
    sa.it_value.tv_usec = 0;
    sa.it_interval.tv_sec = 100;
    sa.it_interval.tv_usec = 0;

    signal(SIGINT, sig_handler);
    signal(SIGALRM, sig_handler);
    setitimer(ITIMER_REAL,&sa,NULL);
    
    dequeueTask = new_dequeue_task(NODE_1, ModuleName[NODE_1], 
                                    NULL, 0, NULL, 0, msgNum, 
                                    sizeof(long), global_cpu_lists[(NODE_1-1)%NR_PROCESSOR]);

    unsigned long start_node_id = NODE_2;
    unsigned long end_node_id = NODE_4;

    for(i=start_node_id; i<=end_node_id; i++) {
        enqueueTask = start_enqueue(i, msgNum, true);
    }

	pthread_setname_np(pthread_self(), "test-1");


    /* 动态删除建立发送队列 */
//    while(1) {
//        sleep(1);
//
//        for(i=start_node_id; i<=end_node_id; i++) {
//            printf(" Delete Q %s\n", ModuleName[i]);
//            VOS_FastQDeleteModule(i);
//        }
//        sleep(1);
//        for(i=start_node_id; i<=end_node_id; i++) {
//            printf(" Create Q %s\n", ModuleName[i]);
//            start_enqueue(i, msgNum, false);
//        }
//        
//        sleep(5);
//    }
    

    pthread_join(dequeueTask, NULL);
    pthread_join(enqueueTask, NULL);

    return EXIT_SUCCESS;
}




