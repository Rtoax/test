/**********************************************************************************************************************\
*  文件： fastq.h
*  介绍： 低时延队列
*  作者： 荣涛
*  日期：
*       2021年1月25日  - 2021年4月19日     
*
* API接口概述
*   
*   VOS_FastQCreateModule   注册消息队列
*   VOS_FastQDump           显示信息
*   VOS_FastQDumpAllModule  显示信息（所有模块）
*   VOS_FastQMsgStatInfo    查询队列内存入队出队信息
*   VOS_FastQSend           发送消息（轮询直至成功发送）
*   VOS_FastQTrySend        发送消息（尝试向队列中插入，当队列满是直接返回false）
*   VOS_FastQRecv           接收消息
*   VOS_FastQMsgNum         获取消息数(需要开启统计功能 _FASTQ_STATS )
*   VOS_FastQAddSet         动态添加 发送接收 set
*   
\**********************************************************************************************************************/

#ifndef __fAStMQ_H
#define __fAStMQ_H 1


#define mod_set                  __mod_set
#define MOD_SET_INITIALIZER     __MOD_SET_INITIALIZER

#define MOD_SETSIZE                    __MOD_SETSIZE
#define MOD_SET(bit, p_mod_set)       __MOD_SET(bit, p_mod_set)
#define MOD_CLR(bit, p_mod_set)       __MOD_CLR(bit, p_mod_set)
#define MOD_ISSET(bit, p_mod_set)     __MOD_ISSET(bit, p_mod_set)
#define MOD_ZERO(p_mod_set)           __MOD_ZERO(p_mod_set)


#ifdef MODULE_ID_MAX // moduleID 最大模块索引值
#define FASTQ_ID_MAX    MODULE_ID_MAX
#else
#define FASTQ_ID_MAX    256
#endif

#include <fastq_types.h>

/**
 *  源模块未初始化时可临时使用的模块ID，只允许使用一次 
 *
 *  目前 0 号 不允许使用
 */
#define VOS_FastQTmpModuleID    0 


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
 *  VOS_FastQCreateModule - 注册消息队列
 *  
 *  param[in]   moduleID    模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   rxset       可能接收对应模块发来的消息 bitmap，见 select() fd_set
 *  param[in]   txset       可能向对应模块发送消息 bitmap，见 select() fd_set
 *  param[in]   msgMax      该模块 的 消息队列 的大小
 *  param[in]   msgSize     最大传递的消息大小
 */
always_inline void inline
VOS_FastQCreateModule(const unsigned long moduleID, 
                         const mod_set *rxset, const mod_set *txset,
                         const unsigned int msgMax, const unsigned int msgSize);


/**
 *  VOS_FastQAddSet - 注册消息队列
 *  
 *  param[in]   moduleID    模块ID， 范围 1 - FASTQ_ID_MAX, 通过 `VOS_FastQCreateModule` 注册的函数
 *  param[in]   rxset       可能接收对应模块发来的消息 bitmap，见 select() fd_set
 *  param[in]   txset       可能向对应模块发送消息 bitmap，见 select() fd_set
 *
 *  注意：这里的`rxset`和`txset`将是`VOS_FastQCreateModule`参数的并集
 */
 always_inline bool inline
VOS_FastQAddSet(const unsigned long moduleID, 
                    const mod_set *rxset, const mod_set *txset);


/**
 *  VOS_FastQDump - 显示信息
 *  
 *  param[in]   fp    文件指针
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline void inline
VOS_FastQDump(FILE*fp, unsigned long moduleID);

/**
 *  VOS_FastQDump - 显示全部模块信息
 *  
 *  param[in]   fp    文件指针
 */
always_inline void inline
VOS_FastQDumpAllModule(FILE*fp);

/**
 *  VOS_FastQMsgStatInfo - 获取统计信息
 *  
 *  param[in]   buf     FastQModuleMsgStatInfo 信息结构体
 *  param[in]   buf_mod_size    buf 信息结构体个数
 *  param[in]   num     函数返回时填回 的 FastQModuleMsgStatInfo 结构个数
 *  param[in]   filter  根据目的和源模块ID进行过滤 详见 fq_module_filter_t
 */

always_inline bool inline
VOS_FastQMsgStatInfo(struct FastQModuleMsgStatInfo *buf, unsigned int buf_mod_size, unsigned int *num, 
                fq_module_filter_t filter);


/**
 *  VOS_FastQSend - 发送消息（轮询直至成功发送）
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
VOS_FastQSend(unsigned int from, unsigned int to, const void *msg, size_t size);


/**
 *  VOS_FastQTrySend - 发送消息（尝试向队列中插入，当队列满是直接返回false）
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
VOS_FastQTrySend(unsigned int from, unsigned int to, const void *msg, size_t size);

/**
 *  VOS_FastQRecv - 接收消息
 *  
 *  param[in]   from    从模块ID from 中读取消息， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   handler 消息处理函数，参照 fq_msg_handler_t 说明
 *
 *  return 成功true 失败false
 *
 *  注意：from 需要使用 FastQCreateModule 注册后使用
 */
always_inline  bool inline
VOS_FastQRecv(unsigned int from, fq_msg_handler_t handler);


/**
 *  VOS_FastQMsgNum - 获取消息数
 *  
 *  param[in]   ID    从模块ID from 中读取消息， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   nr_enqueues 总入队数
 *  param[in]   nr_dequeues 总出队数
 *  param[in]   nr_currents 当前消息数
 *
 *  return 成功true 失败false(不支持, 编译宏控制 _FASTQ_STATS 开启统计功能)
 *
 *  注意：ID 需要使用 FastQCreateModule 注册后使用
 */
always_inline  bool inline
VOS_FastQMsgNum(unsigned int ID, unsigned long *nr_enqueues, unsigned long *nr_dequeues, unsigned long *nr_currents);




/**********************************************************************************************************************\
 **
 **                      从此至该头文件末尾，所有接口禁止应用层使用 
**
\**********************************************************************************************************************/


#define FastQTmpModuleID    VOS_FastQTmpModuleID 



#ifdef _FASTQ_STATS /* 带有统计类的接口 */
//# pragma message "[FastQ] Statistic Class API"
# define VOS_FastQCreateModule(moduleID, rxset, txset, msgMax, msgSize)    FastQCreateModuleStats(moduleID, rxset, txset, msgMax, msgSize, __FILE__, __func__, __LINE__)
# define VOS_FastQAddSet(moduleID, rxset, txset)            FastQAddSetStats(moduleID, rxset, txset)
# define VOS_FastQDump(fp, moduleID)                            FastQDumpStats(fp, moduleID)
# define VOS_FastQDumpAllModule(fp)                            FastQDumpStats(fp, 0)
# define VOS_FastQMsgStatInfo(buf, bufSize, pnum, filter)       FastQMsgStatInfoStats(buf, bufSize, pnum, filter)
# define VOS_FastQSend(moduleSrc, moduleDst, pmsg, msgSize)  FastQSendStats(moduleSrc, moduleDst, pmsg, msgSize)  
# define VOS_FastQTrySend(moduleSrc, moduleDst, pmsg, msgSize)  FastQTrySendStats(moduleSrc, moduleDst, pmsg, msgSize)  
# define VOS_FastQRecv(fromModule, msgHandlerFn)             FastQRecvStats(fromModule, msgHandlerFn)
# define VOS_FastQMsgNum(moduleID, nr_en, nr_de, nt_curr)             FastQMsgNumStats(moduleID, nr_en, nr_de, nt_curr)

#else /* 不带有统计类的接口，时延更低 */
//# pragma message "[FastQ] Low Latency Class API"
# define VOS_FastQCreateModule(moduleID, rxset, txset, msgMax, msgSize)    FastQCreateModule(moduleID, rxset, txset, msgMax, msgSize, __FILE__, __func__, __LINE__)
# define VOS_FastQAddSet(moduleID, rxset, txset)            FastQAddSet(moduleID, rxset, txset)
# define VOS_FastQDump(fp, moduleID)                            FastQDump(fp, moduleID)
# define VOS_FastQDumpAllModule(fp)                            FastQDump(fp, 0)
# define VOS_FastQMsgStatInfo(buf, bufSize, pnum, filter)       FastQMsgStatInfo(buf, bufSize, pnum, filter)
# define VOS_FastQSend(moduleSrc, moduleDst, pmsg, msgSize)  FastQSend(moduleSrc, moduleDst, pmsg, msgSize)  
# define VOS_FastQTrySend(moduleSrc, moduleDst, pmsg, msgSize)  FastQTrySend(moduleSrc, moduleDst, pmsg, msgSize)  
# define VOS_FastQRecv(fromModule, msgHandlerFn)             FastQRecv(fromModule, msgHandlerFn)
# define VOS_FastQMsgNum(moduleID, nr_en, nr_de, nt_curr)             FastQMsgNum(moduleID, nr_en, nr_de, nt_curr)

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
FastQCreateModule(const unsigned long moduleID, 
                        const mod_set *rxset, const mod_set *txset, 
                        const unsigned int msgMax, const unsigned int msgSize, 
                            const char *_file, const char *_func, const int _line);
always_inline void inline
FastQCreateModuleStats(const unsigned long moduleID, 
                        const mod_set *rxset, const mod_set *txset, 
                        const unsigned int msgMax, const unsigned int msgSize, 
                            const char *_file, const char *_func, const int _line);

/**
 *  VOS_FastQAddSet - 注册消息队列
 *  
 *  param[in]   moduleID    模块ID， 范围 1 - FASTQ_ID_MAX, 通过 `VOS_FastQCreateModule` 注册的函数
 *  param[in]   rxset       可能接收对应模块发来的消息 bitmap，见 select() fd_set
 *  param[in]   txset       可能向对应模块发送消息 bitmap，见 select() fd_set
 */
 always_inline bool inline
FastQAddSet(const unsigned long moduleID, 
                    const mod_set *rxset, const mod_set *txset);
 always_inline bool inline
FastQAddSetStats(const unsigned long moduleID, 
                    const mod_set *rxset, const mod_set *txset);

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


/**
 *  VOS_FastQMsgNum - 获取消息数
 *  
 *  param[in]   ID    从模块ID from 中读取消息， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   nr_enqueues 总入队数
 *  param[in]   nr_dequeues 总出队数
 *  param[in]   nr_currents 当前消息数
 *
 *  return 成功true 失败false(不支持, 编译宏控制 _FASTQ_STATS 开启统计功能)
 *
 *  注意：ID 需要使用 FastQCreateModule 注册后使用
 */
always_inline  bool inline
FastQMsgNum(unsigned int ID, unsigned long *nr_enqueues, unsigned long *nr_dequeues, unsigned long *nr_currents);
always_inline  bool inline
FastQMsgNumStats(unsigned int ID, unsigned long *nr_enqueues, unsigned long *nr_dequeues, unsigned long *nr_currents);



#pragma GCC diagnostic pop


#endif /*<__fAStMQ_H>*/


