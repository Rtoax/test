/**********************************************************************************************************************\
*  文件： fastq.h
*  介绍： 低时延队列
*  作者： 荣涛
*  日期：
*       2021年1月25日    创建与开发轮询功能
*       2021年1月27日 添加 通知+轮询 功能接口，消除零消息时的 CPU100% 问题
*       2021年1月28日 调整代码格式，添加必要的注释
*       2021年2月1日 添加多入单出队列功能
*       2021年2月2日 消息队列句柄改成 unsigned long 索引
*       2021年2月3日 统计类接口 和 低时延接口并存
*       2021年2月4日 rt_FastQMsgStatInfo 接口
*
* API接口概述
*   
*   rt_FastQCreateModule   注册消息队列
*   rt_FastQDump           显示信息
*   rt_FastQDumpAllModule  显示信息（所有模块）
*   rt_FastQMsgStatInfo    查询队列内存入队出队信息
*   rt_FastQSend           发送消息（轮询直至成功发送）
*   rt_FastQTrySend        发送消息（尝试向队列中插入，当队列满是直接返回false）
*   rt_FastQRecv           接收消息
*   
\**********************************************************************************************************************/

#ifndef __fAStMQ_H
#define __fAStMQ_H 1

#include <stdbool.h>

#ifndef always_inline
#define always_inline /*__attribute__ ((__always_inline__))*/
#endif


/**
 *  源模块未初始化时可临时使用的模块ID，只允许使用一次 
 */
#define rt_FastQTmpModuleID    0 


/**
 *  FastQModuleMsgStatInfo - 统计信息
 *  
 *  src_module  源模块ID
 *  dst_module  目的模块ID
 *  enqueue     从 src_module 发往 dst_module 的统计， src_module 已发出的消息数
 *  dequeue     从 src_module 发往 dst_module 的统计， dst_module 已接收的消息数
 */
struct FastQModuleMsgStatInfo {
    unsigned long src_module;
    unsigned long dst_module;
    unsigned long enqueue;
    unsigned long dequeue;
};


/**
 *  fq_msg_handler_t - FastQRecvMain 接收函数
 *  
 *  param[in]   msg 接收消息地址
 *  param[in]   sz 接收消息大小，与 FastQCreate (..., msg_size) 保持一致
 */
typedef void (*fq_msg_handler_t)(void*msg, size_t sz);


/**
 *  fq_module_filter_t - 根据目的和源模块ID进行过滤
 *  
 *  param[in]   srcID 源模块ID
 *  param[in]   dstID 目的模块ID
 *
 *  当 fq_module_filter_t 返回 true 时，该 源 到 目的 的消息队列将计入统计
 */
typedef bool (*fq_module_filter_t)(unsigned long srcID, unsigned long dstID);





/**
 *  rt_FastQCreateModule - 注册消息队列
 *  
 *  param[in]   moduleID    模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msgMax      该模块 的 消息队列 的大小
 *  param[in]   msgSize     最大传递的消息大小
 */
always_inline void inline
rt_FastQCreateModule(const unsigned long moduleID, const unsigned int msgMax, const unsigned int msgSize);

/**
 *  rt_FastQDump - 显示信息
 *  
 *  param[in]   fp    文件指针
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline void inline
rt_FastQDump(FILE*fp, unsigned long moduleID);

/**
 *  rt_FastQDump - 显示全部模块信息
 *  
 *  param[in]   fp    文件指针
 */
always_inline void inline
rt_FastQDumpAllModule(FILE*fp);

/**
 *  rt_FastQMsgStatInfo - 获取统计信息
 *  
 *  param[in]   buf     FastQModuleMsgStatInfo 信息结构体
 *  param[in]   buf_mod_size    buf 信息结构体个数
 *  param[in]   num     函数返回时填回 的 FastQModuleMsgStatInfo 结构个数
 *  param[in]   filter  根据目的和源模块ID进行过滤 详见 fq_module_filter_t
 */

always_inline bool inline
rt_FastQMsgStatInfo(struct FastQModuleMsgStatInfo *buf, unsigned int buf_mod_size, unsigned int *num, 
                fq_module_filter_t filter);


/**
 *  rt_FastQSend - 发送消息（轮询直至成功发送）
 *  
 *  param[in]   from    源模块ID， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   to      目的模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msg     传递的消息体
 *  param[in]   size    传递的消息大小
 *
 *  return 成功true （轮询直至发送成功，只可能返回 true ）
 *
 *  注意：from 和 to 需要使用 FastQCreateModule 注册后使用
 */
always_inline bool inline
rt_FastQSend(unsigned int from, unsigned int to, const void *msg, size_t size);


/**
 *  rt_FastQTrySend - 发送消息（尝试向队列中插入，当队列满是直接返回false）
 *  
 *  param[in]   from    源模块ID， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   to      目的模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msg     传递的消息体
 *  param[in]   size    传递的消息大小
 *
 *  return 成功true 失败false
 *
 *  注意：from 和 to 需要使用 FastQCreateModule 注册后使用
 */
always_inline bool inline
rt_FastQTrySend(unsigned int from, unsigned int to, const void *msg, size_t size);

/**
 *  rt_FastQRecv - 接收消息
 *  
 *  param[in]   from    从模块ID from 中读取消息， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   handler 消息处理函数，参照 fq_msg_handler_t 说明
 *
 *  return 成功true 失败false
 *
 *  注意：from 需要使用 FastQCreateModule 注册后使用
 */
always_inline  bool inline
rt_FastQRecv(unsigned int from, fq_msg_handler_t handler);



/**********************************************************************************************************************\
 **
 **                      从此至该头文件末尾，所有接口禁止应用层使用 
**
\**********************************************************************************************************************/
    
#define FastQTmpModuleID    rt_FastQTmpModuleID 



#ifdef _FASTQ_STATS /* 带有统计类的接口 */
//# pragma message "[FastQ] Statistic Class API"
# define rt_FastQCreateModule(moduleID, msgMax, msgSize)    FastQCreateModuleStats(moduleID, msgMax, msgSize, __FILE__, __func__, __LINE__)
# define rt_FastQDump(fp, moduleID)                            FastQDumpStats(fp, moduleID)
# define rt_FastQDumpAllModule(fp)                            FastQDumpStats(fp, 0)
# define rt_FastQMsgStatInfo(buf, bufSize, pnum, filter)       FastQMsgStatInfoStats(buf, bufSize, pnum, filter)
# define rt_FastQSend(moduleSrc, moduleDst, pmsg, msgSize)  FastQSendStats(moduleSrc, moduleDst, pmsg, msgSize)  
# define rt_FastQTrySend(moduleSrc, moduleDst, pmsg, msgSize)  FastQTrySendStats(moduleSrc, moduleDst, pmsg, msgSize)  
# define rt_FastQRecv(fromModule, msgHandlerFn)             FastQRecvStats(fromModule, msgHandlerFn)

#else /* 不带有统计类的接口，时延更低 */
//# pragma message "[FastQ] Low Latency Class API"
# define rt_FastQCreateModule(moduleID, msgMax, msgSize)    FastQCreateModule(moduleID, msgMax, msgSize, __FILE__, __func__, __LINE__)
# define rt_FastQDump(fp, moduleID)                            FastQDump(fp, moduleID)
# define rt_FastQDumpAllModule(fp)                            FastQDump(fp, 0)
# define rt_FastQMsgStatInfo(buf, bufSize, pnum, filter)       FastQMsgStatInfo(buf, bufSize, pnum, filter)
# define rt_FastQSend(moduleSrc, moduleDst, pmsg, msgSize)  FastQSend(moduleSrc, moduleDst, pmsg, msgSize)  
# define rt_FastQTrySend(moduleSrc, moduleDst, pmsg, msgSize)  FastQTrySend(moduleSrc, moduleDst, pmsg, msgSize)  
# define rt_FastQRecv(fromModule, msgHandlerFn)             FastQRecv(fromModule, msgHandlerFn)

#endif


#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

/**
 *  FastQCreateModule - 注册消息队列
 *  
 *  param[in]   moduleID    模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msgMax      该模块 的 消息队列 的大小
 *  param[in]   msgSize     最大传递的消息大小
 */
always_inline void inline
FastQCreateModule(const unsigned long moduleID, const unsigned int msgMax, const unsigned int msgSize, 
                            const char *_file, const char *_func, const int _line);
always_inline void inline
FastQCreateModuleStats(const unsigned long moduleID, const unsigned int msgMax, const unsigned int msgSize, 
                            const char *_file, const char *_func, const int _line);


/**
 *  FastQDump - 显示信息
 *  
 *  param[in]   fp    文件指针
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline void inline
FastQDump(FILE*fp, unsigned long module_id);
always_inline void inline
FastQDumpStats(FILE*fp, unsigned long module_id);


/**
 *  FastQMsgStatInfo - 获取统计信息
 *  
 *  param[in]   buf    FastQModuleMsgStatInfo 信息结构体
 *  param[in]   buf_mod_size    buf 信息结构体个数
 *  param[in]   num 函数返回时填回 的 FastQModuleMsgStatInfo 结构个数
 *  param[in]   filter    根据目的和源模块ID进行过滤 详见 fq_module_filter_t
 */

always_inline bool inline
FastQMsgStatInfo(struct FastQModuleMsgStatInfo *buf, unsigned int buf_mod_size, unsigned int *num, 
                fq_module_filter_t filter);
always_inline bool inline
FastQMsgStatInfoStats(struct FastQModuleMsgStatInfo *buf, unsigned int buf_mod_size, unsigned int *num, 
                fq_module_filter_t filter);



/**
 *  FastQSend - 发送消息（轮询直至成功发送）
 *  
 *  param[in]   from    源模块ID， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   to      目的模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msg     传递的消息体
 *  param[in]   size    传递的消息大小
 *
 *  return 成功true （轮询直至发送成功，只可能返回 true ）
 *
 *  注意：from 和 to 需要使用 FastQCreateModule 注册后使用
 */
always_inline bool inline
FastQSend(unsigned int from, unsigned int to, const void *msg, size_t size);
always_inline bool inline
FastQSendStats(unsigned int from, unsigned int to, const void *msg, size_t size);


/**
 *  FastQTrySend - 发送消息（尝试向队列中插入，当队列满是直接返回false）
 *  
 *  param[in]   from    源模块ID， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   to      目的模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msg     传递的消息体
 *  param[in]   size    传递的消息大小
 *
 *  return 成功true 失败false
 *
 *  注意：from 和 to 需要使用 FastQCreateModule 注册后使用
 */
always_inline bool inline
FastQTrySend(unsigned int from, unsigned int to, const void *msg, size_t size);
always_inline bool inline
FastQTrySendStats(unsigned int from, unsigned int to, const void *msg, size_t size);

/**
 *  FastQRecv - 接收消息
 *  
 *  param[in]   from    从模块ID from 中读取消息， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   handler 消息处理函数，参照 fq_msg_handler_t 说明
 *
 *  return 成功true 失败false
 *
 *  注意：from 需要使用 FastQCreateModule 注册后使用
 */
always_inline  bool inline
FastQRecv(unsigned int from, fq_msg_handler_t handler);
always_inline  bool inline
FastQRecvStats(unsigned int from, fq_msg_handler_t handler);


#pragma GCC diagnostic pop


#endif /*<__fAStMQ_H>*/


