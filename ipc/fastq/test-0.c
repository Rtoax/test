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

#include <fastq.h>

#include "common.h"


int sig_handler(int signum) {

    FastQDump(NULL, NODE_1);
    exit(1);
}

int main()
{
    int max_msg = 16;
    
    signal(SIGINT, sig_handler);
    
    FastQCreateModule(NODE_1, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModule(NODE_2, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModule(NODE_3, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModule(NODE_4, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    FastQCreateModule(NODE_4, max_msg, sizeof(unsigned long), __FILE__, __func__, __LINE__);
    

    return EXIT_SUCCESS;
}




