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

//#define _FASTQ_STATS //开启统计功能
#include <fastq.h>

#include "common.h"

#define NR_PROCESSOR sysconf(_SC_NPROCESSORS_ONLN)

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
    unsigned long srcModuleID = parg->srcModuleId;
    unsigned long dstModuleId = parg->dstModuleId;
    
    while(1) {
        pmsg = &ptest_msg[i++%TEST_NUM];
        pmsg->latency = RDTSC();
        unsigned long addr = (unsigned long)pmsg;

        switch(pmsg->latency%4) {
            case 0:
                VOS_FastQSend(srcModuleID, dstModuleId, &addr, sizeof(unsigned long));
                break;
            case 1:
                VOS_FastQTrySend(srcModuleID, dstModuleId, &addr, sizeof(unsigned long));
                break;
            case 2:
                VOS_FastQSendByName(ModuleName[srcModuleID], ModuleName[dstModuleId], &addr, sizeof(unsigned long));
                break;
            case 3:
                VOS_FastQTrySendByName(ModuleName[srcModuleID], ModuleName[dstModuleId], &addr, sizeof(unsigned long));
                break;
        }
        
//        printf("send: %x,(%x)\n", pmsg->magic, TEST_MSG_MAGIC);
//        sleep(1);
        send_cnt++;
        
        if(send_cnt % 5000000 == 0) {
            printf("enqueue sleep().\n");
            sleep(5);
        }
    }
    assert(0);
    pthread_exit(NULL);
}


void handler_test_msg(void* msg, size_t size)
{
    unsigned long addr =  *(unsigned long*)msg;
    test_msgs_t *pmsg;

    pmsg = (test_msgs_t *)addr;
//    printf("recv: %x,(%x)\n", pmsg->magic, TEST_MSG_MAGIC);
    latency_total += RDTSC() - pmsg->latency;
    pmsg->latency = 0;
    if(pmsg->magic != TEST_MSG_MAGIC) {
        error_msgs++;
    }
    
    total_msgs++;
    
    if(total_msgs % 50000 == 0) {
        printf("dequeue. per msgs \033[1;31m%lf ns\033[m, msgs (total %ld, err %ld).\n", 
                latency_total*1.0/total_msgs/3000000000*1000000000,
                total_msgs, error_msgs);
    }

}

void *dequeue_task(void*arg) {
    struct dequeue_arg *parg = (struct dequeue_arg*)arg;
    reset_self_cpuset(parg->cpu_list);
    
    VOS_FastQRecvByName( ModuleName[parg->srcModuleId], handler_test_msg);
    pthread_exit(NULL);
}


pthread_t new_enqueue_task(unsigned long moduleId, unsigned long dstModuleId, char *moduleName, 
                               unsigned long *RxArray, int nRx,
                               unsigned long *TxArray, int nTx,
                               size_t maxMsg, size_t msgSize,
                               const char *cpulist) {
    pthread_t task;
    unsigned int i =0;
    struct enqueue_arg *enqueueArg = malloc(sizeof(struct enqueue_arg));

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

    
    test_msgs_t *test_msg = (test_msgs_t *)malloc(sizeof(test_msgs_t)*TEST_NUM);
    for(i=0;i<TEST_NUM;i++) {
        test_msg->magic = TEST_MSG_MAGIC + (i%10000==0?1:0); //有比特错误的消息
//        test_msg[i].magic = TEST_MSG_MAGIC; 
        test_msg[i].value = 0xff00000000000000 + i+1;
    }

    enqueueArg->srcModuleId = moduleId;
    enqueueArg->dstModuleId = dstModuleId;
    enqueueArg->cpu_list = strdup(cpulist);
    enqueueArg->msgs = test_msg;

    pthread_create(&task, NULL, enqueue_task, enqueueArg);
	pthread_setname_np(task, moduleName?moduleName:"enqueue");

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


int main()
{
    pthread_t dequeueTask;
    pthread_t enqueueTask;
    unsigned long i;
    unsigned long TXarr[] = {NODE_1};

    struct itimerval sa;
    sa.it_value.tv_sec = 10;
    sa.it_value.tv_usec = 0;
    sa.it_interval.tv_sec = 10;
    sa.it_interval.tv_usec = 0;

    signal(SIGINT, sig_handler);
    signal(SIGALRM, sig_handler);
    setitimer(ITIMER_REAL,&sa,NULL);

    dequeueTask = new_dequeue_task(NODE_1, ModuleName[NODE_1], 
                                    NULL, 0, NULL, 0, 16, 
                                    sizeof(test_msgs_t), global_cpu_lists[NODE_1-1]);

    for(i=NODE_2; i<=NODE_5; i++) {
        enqueueTask = new_enqueue_task(i, NODE_1, ModuleName[i], 
                                        NULL, 0, TXarr, 1, 
                                        16, sizeof(test_msgs_t), 
                                        global_cpu_lists[(i-1)%NR_PROCESSOR]);

    }
	pthread_setname_np(pthread_self(), "test-1");

    pthread_join(dequeueTask, NULL);
    pthread_join(enqueueTask, NULL);

    return EXIT_SUCCESS;
}




