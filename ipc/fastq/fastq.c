/**********************************************************************************************************************\
*  文件： fastq.c
*  介绍： 低时延队列
*  作者： 荣涛
*  日期：
*       2021年1月25日    创建与开发轮询功能
*       2021年1月27日 添加 通知+轮询 功能接口，消除零消息时的 CPU100% 问题
*       2021年1月28日 调整代码格式，添加必要的注释
*       2021年2月1日 添加多入单出队列功能 ： 采用 epoll 实现
*       2021年2月2日 添加统计功能接口，尽可能减少代码量
*       2021年2月3日 统计类接口 和 低时延接口并存
*       2021年3月2日 为满足实时内核，添加 select()，同时支持 epoll()
*       2021年3月3日 统计类接口 和 低时延接口并存
*       2021年3月4日 rt_FastQMsgStatInfo 接口
*       2021年4月7日 添加模块掩码，限制底层创建 fd 数量
*
\**********************************************************************************************************************/
#include <stdint.h>
#include <assert.h>
    
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/eventfd.h> //eventfd
#include <sys/select.h> //FD_SETSIZE
#include <sys/epoll.h>
#include <pthread.h>

#include "fastq.h"


#if (!defined(__i386__) && !defined(__x86_64__))
# error Unsupported CPU
#endif



/* 多路复用器 的选择， 默认采用 select()
 *  epoll 实时内核对epoll影响非常严重，详情请见 Sys_epoll_wait->spin_lock_local
 *  select 实时内核对 epoll 影响不严重
 */
#if defined(_FASTQ_EPOLL) && defined(_FASTQ_SELECT)
# error "You must choose one of selector from _FASTQ_EPOLL or _FASTQ_SELECT"
#endif

#if !defined(_FASTQ_EPOLL) && !defined(_FASTQ_SELECT)
# define _FASTQ_SELECT 1 //使用 select()
#endif

#if !defined(_FASTQ_SELECT)
# if !defined(_FASTQ_EPOLL)
#  define _FASTQ_EPOLL 1
# endif
#endif

//#ifdef _FASTQ_EPOLL
//# error _FASTQ_EPOLL
//#endif
//#ifdef _FASTQ_SELECT
//# error _FASTQ_SELECT
//#endif

/**
 *  内存分配器接口
 */
#define FastQMalloc(size) malloc(size)
#define FastQStrdup(str) strdup(str)
#define FastQFree(ptr) free(ptr)


#ifndef likely
#define likely(x)    __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#ifndef cachelinealigned
#define cachelinealigned __attribute__((aligned(64)))
#endif

#ifndef _unused
#define _unused             __attribute__((unused))
#endif

//#define FASTQ_DEBUG
#ifdef FASTQ_DEBUG
#define LOG_DEBUG(fmt...)  do{printf("\033[33m[%s:%d]", __func__, __LINE__);printf(fmt);printf("\033[m");}while(0)
#else
#define LOG_DEBUG(fmt...) 
#endif

/**
 * The atomic counter structure.
 */
typedef struct {
	volatile int64_t cnt;  /**< Internal counter value. */
} atomic64_t;

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"



// 内存屏障
always_inline static void  inline _unused mbarrier() { asm volatile("": : :"memory"); }
// This version requires SSE capable CPU.
always_inline static void  inline _unused mrwbarrier() { asm volatile("mfence":::"memory"); }
always_inline static void  inline _unused mrbarrier()  { asm volatile("lfence":::"memory"); }
always_inline static void  inline _unused mwbarrier()  { asm volatile("sfence":::"memory"); }
always_inline static void  inline _unused __relax()  { asm volatile ("pause":::"memory"); } 
always_inline static void  inline _unused __lock()   { asm volatile ("cli" ::: "memory"); }
always_inline static void  inline _unused __unlock() { asm volatile ("sti" ::: "memory"); }



static inline int always_inline _unused 
atomic64_cmpset(volatile uint64_t *dst, uint64_t exp, uint64_t src)
{
	uint8_t res;

	asm volatile(
			"lock ; "
			"cmpxchgq %[src], %[dst];"
			"sete %[res];"
			: [res] "=a" (res),     /* output */
			  [dst] "=m" (*dst)
			: [src] "r" (src),      /* input */
			  "a" (exp),
			  "m" (*dst)
			: "memory");            /* no-clobber list */

	return res;
}

static inline uint64_t always_inline _unused
atomic64_exchange(volatile uint64_t *dst, uint64_t val)
{
	asm volatile(
			"lock ; "
			"xchgq %0, %1;"
			: "=r" (val), "=m" (*dst)
			: "0" (val),  "m" (*dst)
			: "memory");         /* no-clobber list */
	return val;
}

static inline void always_inline _unused
atomic64_init(atomic64_t *v)
{
	atomic64_cmpset((volatile uint64_t *)&v->cnt, v->cnt, 0);
}

static inline int64_t always_inline _unused
atomic64_read(atomic64_t *v)
{
    return v->cnt;
}

static inline void always_inline _unused
atomic64_set(atomic64_t *v, int64_t new_value)
{
    atomic64_cmpset((volatile uint64_t *)&v->cnt, v->cnt, new_value);
}

static inline void always_inline _unused
atomic64_add(atomic64_t *v, int64_t inc)
{
	asm volatile(
			"lock ; "
			"addq %[inc], %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: [inc] "ir" (inc),     /* input */
			  "m" (v->cnt)
			);
}

static inline void always_inline _unused
atomic64_sub(atomic64_t *v, int64_t dec)
{
	asm volatile(
			"lock ; "
			"subq %[dec], %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: [dec] "ir" (dec),     /* input */
			  "m" (v->cnt)
			);
}

static inline void always_inline _unused
atomic64_inc(atomic64_t *v)
{
	asm volatile(
			"lock ; "
			"incq %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: "m" (v->cnt)          /* input */
			);
}

static inline void always_inline _unused
atomic64_dec(atomic64_t *v)
{
	asm volatile(
			"lock ; "
			"decq %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: "m" (v->cnt)          /* input */
			);
}

static inline int64_t always_inline _unused
atomic64_add_return(atomic64_t *v, int64_t inc)
{
	int64_t prev = inc;

	asm volatile(
			"lock ; "
			"xaddq %[prev], %[cnt]"
			: [prev] "+r" (prev),   /* output */
			  [cnt] "=m" (v->cnt)
			: "m" (v->cnt)          /* input */
			);
	return prev + inc;
}

static inline int64_t always_inline _unused
atomic64_sub_return(atomic64_t *v, int64_t dec)
{
	return atomic64_add_return(v, -dec);
}

static inline int always_inline _unused
atomic64_inc_and_test(atomic64_t *v)
{
	uint8_t ret;

	asm volatile(
			"lock ; "
			"incq %[cnt] ; "
			"sete %[ret]"
			: [cnt] "+m" (v->cnt), /* output */
			  [ret] "=qm" (ret)
			);

	return ret != 0;
}

static inline int always_inline _unused
atomic64_dec_and_test(atomic64_t *v)
{
	uint8_t ret;

	asm volatile(
			"lock ; "
			"decq %[cnt] ; "
			"sete %[ret]"
			: [cnt] "+m" (v->cnt),  /* output */
			  [ret] "=qm" (ret)
			);
	return ret != 0;
}

static inline int always_inline _unused
atomic64_test_and_set(atomic64_t *v)
{
	return atomic64_cmpset((volatile uint64_t *)&v->cnt, 0, 1);
}

static inline void always_inline _unused
atomic64_clear(atomic64_t *v)
{
	atomic64_set(v, 0);
}


always_inline static unsigned int  _unused
__power_of_2(unsigned int size) {
    unsigned int i;
    for (i=0; (1U << i) < size; i++);
    return 1U << i;
}


#ifdef _FQ_NAME
#error You gotta be kidding me, do not define _FQ_NAME.
#endif


FILE* fastq_log_fp = NULL;

#define fastq_log(fmt...) do{\
                    fprintf(fastq_log_fp, fmt); \
                    fflush(fastq_log_fp); \
                }while(0)

#ifndef _fastq_fprintf
#define _fastq_fprintf(fp, fmt...) do{   \
                    fastq_log(fmt);     \
                    LOG_DEBUG(fmt);     \
                    fprintf(fp, fmt);   \
                }while(0)
#endif

static void __attribute__((constructor(101))) __fastq_log_init() {
    char fasgq_log_file[256] = {"./fastq.log"};
    fastq_log_fp = fopen(fasgq_log_file, "w");
    fastq_log_fp = fastq_log_fp?fastq_log_fp:stderr;
}


/**********************************************************************************************************************
 *  原始接口
 **********************************************************************************************************************/
#define _FQ_NAME(name)   name

#include "fastq_compat.c"

/***********************************************************************************************************************
 *  支持统计功能的接口
 **********************************************************************************************************************/
#undef _FQ_NAME
#define _FQ_NAME(name)   name##Stats
#define FASTQ_STATISTICS //统计功能

#include "fastq_compat.c"


#pragma GCC diagnostic pop

