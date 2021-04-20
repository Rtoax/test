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
*       2021年3月4日 VOS_FastQMsgStatInfo 接口
*       2021年4月7日 添加模块掩码，限制底层创建 fd 数量
*       2021年4月19日 获取当前队列消息数    (需要开启统计功能 _FASTQ_STATS )
*                     动态添加 发送接收 set
*                     模块名索引 发送接口(明天写接收接口)
*       2021年4月20日 模块名索引
*                     接口统一，只支持统计类接口
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

#include <fastq.h>

#include <dict.h>   //哈希查找 模块名 -> moduleID
#include <sds.h>

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
# define _FASTQ_SELECT 1 //默认使用 select()
#endif

#if !defined(_FASTQ_SELECT)
# if !defined(_FASTQ_EPOLL)
#  define _FASTQ_EPOLL 1
# endif
#endif

/**
 *  内存分配器接口
 */
#define FastQMalloc(size)   malloc(size)
#define FastQStrdup(str)    strdup(str)
#define FastQFree(ptr)      free(ptr)


#ifndef likely
#define likely(x)    __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#ifndef __cachelinealigned
#define __cachelinealigned __attribute__((aligned(64)))
#endif

#ifndef weak_alias
#define weak_alias(name, aliasname) extern typeof (name) aliasname __attribute__ ((weak, alias(#name)))
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


typedef enum  {
    MODULE_STATUS_INVALIDE,
    MODULE_STATUS_REGISTED,
    MODULE_STATUS_MODIFY,
    MODULE_STATUS_OK = MODULE_STATUS_REGISTED, //必须相等
}module_status_t;

// FastQRing
struct FastQRing {
    unsigned long src;  //是 1- FASTQ_ID_MAX 的任意值
    unsigned long dst;  //是 1- FASTQ_ID_MAX 的任意值 二者不能重复

    //统计字段
    struct {
        atomic64_t nr_enqueue;
        atomic64_t nr_dequeue;
        
    }__cachelinealigned;
    
    unsigned int _size;
    size_t _msg_size;
    char _pad1[64];
    volatile unsigned int _head;
    char _pad2[64];    
    volatile unsigned int _tail;
    char _pad3[64];    
    int _evt_fd;        //队列eventfd通知
    char _ring_data[];  //保存实际对象
}__cachelinealigned;


//模块
struct FastQModule {
#define NAME_LEN 64
    char name[NAME_LEN];          //模块名
    bool already_register;  //true - 已注册, other - 没注册
    module_status_t status;
    union {
        int epfd;   //epoll fd
        struct {    //select
            int maxfd;
            pthread_rwlock_t rwlock;    //保护 fd_set
            fd_set readset, allset;
            int producer[FD_SETSIZE];
        } selector;
    };
    unsigned long module_id;//是 1- FASTQ_ID_MAX 的任意值
    unsigned int ring_size; //队列大小，ring 节点数
    unsigned int msg_size;  //消息大小， ring 节点大小
    
    char *_file;    //调用注册函数的 文件名
    char *_func;    //调用注册函数的 函数名
    int _line;      //调用注册函数的 文件中的行号

    struct {
        pthread_rwlock_t rwlock;    //保护 mod_set
        mod_set set;//具体的 bitmap
    }rx, tx;        //发送和接收
    struct FastQRing **_ring;
}__cachelinealigned;


static uint64_t _unused inline dictSdsCaseHash(const void *key) {
    return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}
static void _unused inline dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

/* A case insensitive version used for the command lookup table and other
 * places where case insensitive non binary-safe comparison is needed. */
static int _unused inline dictSdsKeyCaseCompare(void *privdata, const void *key1,
        const void *key2)
{
    DICT_NOTUSED(privdata);

    return strcasecmp(key1, key2) == 0;
}

/* Command table. sds string -> command struct pointer. */
static dictType _unused commandTableDictType = {
    dictSdsCaseHash,            /* hash function */
    NULL,                       /* key dup */
    NULL,                       /* val dup */
    dictSdsKeyCaseCompare,      /* key compare */
    dictSdsDestructor,          /* key destructor */
    NULL,                       /* val destructor */
    NULL                        /* allow to expand */
};

static void _unused inline dict_register_module(dict *d, char *name, unsigned long id) {
    int ret = dictAdd(d, name, (void*)id);
    if(ret != DICT_OK) {
        assert(ret==DICT_OK && "Your Module's name is invalide.\n");
    }
}

static unsigned long inline _unused dict_find_module_id_byname(dict *d, char *name) {
    dictEntry *entry = dictFind(d, name);
    return (unsigned long)dictGetVal(entry);
}


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

FILE* fastq_log_fp = NULL;

static struct FastQModule *_AllModulesRings = NULL;
static pthread_rwlock_t _AllModulesRingsLock = PTHREAD_RWLOCK_INITIALIZER; //只在注册时保护使用
static dict *dictModuleNameID = NULL;

// 从 event fd 查找 ring 的最快方法
static  struct {
    struct FastQRing *tlb_ring;
}__cachelinealigned _evtfd_to_ring[FD_SETSIZE] = {{NULL}};


static void  __fastq_log_init() {
    char fasgq_log_file[256] = {"./fastq.log"};
    fastq_log_fp = fopen(fasgq_log_file, "w");
    fastq_log_fp = fastq_log_fp?fastq_log_fp:stderr;
}

/**
 *  FastQ 初始化 函数，初始化 _AllModulesRings 全局变量
 */
static void __attribute__((constructor(105))) __FastQInitCtor() {
    int i, j;
    LOG_DEBUG("Init _module_src_dst_to_ring.\n");

    __fastq_log_init();
    
    _AllModulesRings = FastQMalloc(sizeof(struct FastQModule)*(FASTQ_ID_MAX+1));
    
    for(i=0; i<=FASTQ_ID_MAX; i++) {

        struct FastQModule *this_module = &_AllModulesRings[i];
    
        __atomic_store_n(&this_module->already_register, false, __ATOMIC_RELEASE);
        __atomic_store_n(&this_module->status, MODULE_STATUS_INVALIDE, __ATOMIC_RELEASE);
    
        this_module->module_id = i;
    
#if defined(_FASTQ_EPOLL)
        
        this_module->epfd = -1;

#elif defined(_FASTQ_SELECT)

        FD_ZERO(&this_module->selector.allset);
        FD_ZERO(&this_module->selector.readset);
        this_module->selector.maxfd    = 0;

        for(j=0; j<FD_SETSIZE; ++j)
        {
            this_module->selector.producer[j] = -1;
        }
        pthread_rwlock_init(&this_module->selector.rwlock, NULL);
    
#endif
        //清空   rx 和 tx set
        MOD_ZERO(&this_module->rx.set);
        MOD_ZERO(&this_module->tx.set);
        pthread_rwlock_init(&this_module->rx.rwlock, NULL);
        pthread_rwlock_init(&this_module->tx.rwlock, NULL);

        //清空 ring
        this_module->ring_size = 0;
        this_module->msg_size = 0;

        //分配所有 ring 指针
        struct FastQRing **___ring = FastQMalloc(sizeof(struct FastQRing*)*(FASTQ_ID_MAX+1));
        assert(___ring && "Malloc Failed: Out of Memory.");
        
        this_module->_ring = ___ring;
        for(j=0; j<=FASTQ_ID_MAX; j++) { 
	        __atomic_store_n(&this_module->_ring[j], NULL, __ATOMIC_RELAXED);
        }
    }
    
    //初始化 模块名->模块ID 字典
    dictModuleNameID = dictCreate(&commandTableDictType,NULL);
    dictExpand(dictModuleNameID, FASTQ_ID_MAX);
}


/**********************************************************************************************************************
 *  原始接口
 **********************************************************************************************************************/

static void inline
__fastq_create_ring(struct FastQModule *pmodule, const unsigned long src, const unsigned long dst) {

    const unsigned int ring_size = pmodule->ring_size;
    const unsigned int msg_size = pmodule->msg_size;

    fastq_log("Create ring : src(%lu)->dst(%lu) ringsize(%d) msgsize(%d).\n", src, dst, ring_size, msg_size);

    unsigned long ring_real_size = sizeof(struct FastQRing) + ring_size*(msg_size+sizeof(size_t));
                      
    struct FastQRing *new_ring = FastQMalloc(ring_real_size);
    assert(new_ring && "Allocate FastQRing Failed. (OOM error)");
    
    memset(new_ring, 0x00, ring_real_size);

    LOG_DEBUG("Create ring %ld->%ld.\n", src, dst);
    new_ring->src = src;
    new_ring->dst = dst;
    new_ring->_size = ring_size - 1;
    
    new_ring->_msg_size = msg_size + sizeof(size_t);
    new_ring->_evt_fd = eventfd(0, EFD_CLOEXEC);
    assert(new_ring->_evt_fd && "Too much eventfd called, no fd to use."); //都TMD没有fd了，你也是厉害
    
    LOG_DEBUG("Create module %ld event fd = %d.\n", dst, new_ring->_evt_fd);
        
    /* fd->ring 的快表 更应该是空的 */
    if (likely(__atomic_load_n(&_evtfd_to_ring[new_ring->_evt_fd].tlb_ring, __ATOMIC_RELAXED) == NULL)) {
        __atomic_store_n(&_evtfd_to_ring[new_ring->_evt_fd].tlb_ring, new_ring, __ATOMIC_RELAXED);
    } else {
        fastq_log("Already Register src(%ul)->dst(%ul) module.", src, dst);
        assert(0 && "Already Register.");
    }
    
#if defined(_FASTQ_EPOLL)

    struct epoll_event event;
    event.data.fd = new_ring->_evt_fd;
    event.events = EPOLLIN; //必须采用水平触发
    epoll_ctl(pmodule->epfd, EPOLL_CTL_ADD, event.data.fd, &event);
    LOG_DEBUG("Add eventfd %d to epollfd %d.\n", new_ring->_evt_fd, pmodule->epfd);
    
#elif defined(_FASTQ_SELECT)
    pthread_rwlock_wrlock(&pmodule->selector.rwlock);
    FD_SET(new_ring->_evt_fd, &pmodule->selector.allset);    
    if(new_ring->_evt_fd > pmodule->selector.maxfd)
    {
        pmodule->selector.maxfd = new_ring->_evt_fd;
    }
    pthread_rwlock_unlock(&pmodule->selector.rwlock);
#endif

    //统计功能
    atomic64_init(&new_ring->nr_dequeue);
    atomic64_init(&new_ring->nr_enqueue);

    __atomic_store_n(&pmodule->_ring[src], new_ring, __ATOMIC_RELAXED);
}


always_inline void inline
FastQCreateModule(const char *name, const unsigned long module_id, 
                     const mod_set *rxset, const mod_set *txset, 
                     const unsigned int ring_size, const unsigned int msg_size, 
                            const char *_file, const char *_func, const int _line) {
    assert(module_id <= FASTQ_ID_MAX && "Module ID out of range");

    if(unlikely(!_file) || unlikely(!_func) || unlikely(_line <= 0) || unlikely(!name)) {
        assert(0 && "NULL pointer error");
    }
    
    int i;

    struct FastQModule *this_module = &_AllModulesRings[module_id];

    //锁
    pthread_rwlock_wrlock(&_AllModulesRingsLock);
    
    //检查模块是否已经注册 并 设置已注册标志
    bool after_status = false;
    if(!__atomic_compare_exchange_n(&this_module->already_register, &after_status, 
                                        true, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
        _fastq_fprintf(stderr, "\033[1;5;31mModule ID %ld already register in file <%s>'s function <%s> at line %d\033[m\n", \
                        module_id,
                        this_module->_file,
                        this_module->_func,
                        this_module->_line);
        
        pthread_rwlock_unlock(&_AllModulesRingsLock);
        assert(0 && "ERROR: Already register module.");
        return ;
    }

    /**
     *  从这里开始, `this_module`的访问线程安全
     */
    __atomic_store_n(&this_module->status, MODULE_STATUS_REGISTED, __ATOMIC_RELEASE);   //已注册
    __atomic_store_n(&this_module->status, MODULE_STATUS_MODIFY, __ATOMIC_RELEASE);     //在修改

    //保存名字并添加至 字典
    strncpy(this_module->name, name, NAME_LEN);
    dict_register_module(dictModuleNameID, name, module_id);
    
        
    //设置 发送 接收 set
    if(rxset) {
        pthread_rwlock_wrlock(&this_module->rx.rwlock);
        memcpy(&this_module->rx.set, rxset, sizeof(mod_set));
        pthread_rwlock_unlock(&this_module->rx.rwlock);
    } 
    if(txset) {
        pthread_rwlock_wrlock(&this_module->tx.rwlock);
        memcpy(&this_module->tx.set, txset, sizeof(mod_set));
        pthread_rwlock_unlock(&this_module->tx.rwlock);
    }
    
#if defined(_FASTQ_EPOLL)
    this_module->epfd = epoll_create(1);
    assert(this_module->epfd && "Epoll create error");
    _fastq_fprintf(stderr, "Create FastQ module with epoll, moduleID = %ld.\n", module_id);
#elif defined(_FASTQ_SELECT)
    _fastq_fprintf(stderr, "Create FastQ module with select, moduleID = %ld.\n", module_id);
#endif

    //在哪里注册，用于调试
    this_module->_file = FastQStrdup(_file);
    this_module->_func = FastQStrdup(_func);
    this_module->_line = _line;
    
    //队列大小
    this_module->ring_size = __power_of_2(ring_size);
    this_module->msg_size = msg_size;

    //当设置了标志位，并且对应的 ring 为空
    if(MOD_ISSET(0, &this_module->rx.set) && 
        !__atomic_load_n(&this_module->_ring[0], __ATOMIC_RELAXED)) {
        /* 当源模块未初始化时又想向目的模块发送消息 */
        __fastq_create_ring(this_module, 0, module_id);
    }
    /*建立住的模块和其他模块的连接关系
        若注册前的连接关系如下：
        下图为已经注册过两个模块 (模块 A 和 模块 B) 的数据结构
                    +---+
                    |   |
                    | A |
                    |   |
                  / +---+
                 /  /
                /  /
               /  /
              /  /
             /  /
         +---+ /
         |   |
         | B |
         |   |
         +---+

        在此基础上注册新的模块 (模块 C) 通过接下来的操作，将会创建四个 ring

                    +---+
                    |   |
                    | A |
                    |   |
                  / +---+ \
                 /  /   \  \
                /  /     \  \
               /  /       \  \
              /  /         \  \
             /  /           \  \
         +---+ /             \ +---+ 
         |   | <-------------- |   |
         | B |                 | C |
         |   | --------------> |   |
         +---+                 +---+

        值得注意的是，在创建 ring 时，会根据入参中的 rxset 和 txset 决定分配哪些 ring 和 eventfd
        以上面的 ABC 模块为例，具体如下：

        注册过程：假设模块ID分别为 A=1, B=2, C=3
        
                    rxset       txset           表明
            A(1)    0110        0000    A可能从B(2)C(3)接收，不接收任何数据
            B(2)    0000        0010    B不接受任何数据，可能向A(1)发送
            C(3)    0000        0010    C不接受任何数据，可能向A(1)发送
         那么创建的 底层数据结构将会是
         
                    +---+
                    |   |
                    | A |
                    |   |
                  /`+---+ \`
                 /         \
                /           \
               /             \
              /               \
             /                 \
         +---+                 +---+ 
         |   |                 |   |
         | B |                 | C |
         |   |                 |   |
         +---+                 +---+
        
    */
    for(i=1; i<=FASTQ_ID_MAX; i++) {
        
        if(i==module_id) continue;

        struct FastQModule *peer_module = &_AllModulesRings[i];
    
        if(!__atomic_load_n(&peer_module->already_register, __ATOMIC_RELAXED)) {
            continue;
        }

        //任意一个模块标记了可能发送或者接收的模块，都将创建队列
        if(MOD_ISSET(i, &this_module->rx.set) || 
           MOD_ISSET(module_id, &peer_module->tx.set)) {

           if(!MOD_ISSET(i, &this_module->rx.set)) {
                MOD_SET(i, &this_module->rx.set);
           }
           if(!MOD_ISSET(module_id, &peer_module->tx.set)) {
                MOD_SET(module_id, &peer_module->tx.set);
           }
           __fastq_create_ring(this_module, i, module_id);
        }
        if(!peer_module->_ring[module_id]) {
            
            if(MOD_ISSET(i, &this_module->tx.set) || 
               MOD_ISSET(module_id, &peer_module->rx.set)) {

                if(!MOD_ISSET(i, &this_module->tx.set)) {
                    MOD_SET(i, &this_module->tx.set);
                }
                if(!MOD_ISSET(module_id, &peer_module->rx.set)) {
                    MOD_SET(module_id, &peer_module->rx.set);
                }
               
                __fastq_create_ring(peer_module, module_id, i);
            }
        }
    }
    
    pthread_rwlock_unlock(&_AllModulesRingsLock);
    __atomic_store_n(&this_module->status, MODULE_STATUS_REGISTED, __ATOMIC_RELEASE);   //已注册

    return;
}


always_inline bool inline
FastQAddSet(const unsigned long moduleID, const mod_set *rxset, const mod_set *txset) {
                                
    if(unlikely(!rxset && !txset)) {
        return false;
    }
    if(moduleID <= 0 || moduleID > FASTQ_ID_MAX) {
        return false;
    }
    struct FastQModule *this_module = &_AllModulesRings[moduleID];
    
    if(!__atomic_load_n(&this_module->already_register, __ATOMIC_RELAXED)) {
        
        return false;
    }

    //自旋获取修改权限
    module_status_t should_be = MODULE_STATUS_OK;
    while(!__atomic_compare_exchange_n(&this_module->status, &should_be, 
                                        MODULE_STATUS_MODIFY, 0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)) {
        _fastq_fprintf(stderr, "Add FastQ module error moduleID = %ld, status wrong.\n", moduleID);
        __relax();
        should_be = MODULE_STATUS_OK;
    }
    
    /**
     *  从这里开始, 将会修改 模块内容，模块状态为 `MODULE_STATUS_MODIFY`
     */
    
    int i;
    _fastq_fprintf(stderr, "Add FastQ module set to moduleID = %ld.\n", moduleID);
    
    pthread_rwlock_wrlock(&_AllModulesRingsLock);

    //遍历
    for(i=1; i<=FASTQ_ID_MAX; i++) {
        if(i==moduleID) continue;
        
        struct FastQModule *peer_module = &_AllModulesRings[i];
        
        //目的模块必须存在
        if(!__atomic_load_n(&peer_module->already_register, __ATOMIC_RELAXED)) {
            continue;
        }

        //接收
        pthread_rwlock_wrlock(&this_module->rx.rwlock);
        if(rxset && MOD_ISSET(i, rxset) && !MOD_ISSET(i, &this_module->rx.set)) {
            MOD_SET(i, &this_module->rx.set);
            pthread_rwlock_wrlock(&peer_module->tx.rwlock);
            MOD_SET(moduleID, &peer_module->tx.set);
            pthread_rwlock_unlock(&peer_module->tx.rwlock);

            __fastq_create_ring(this_module, i, moduleID);
        }
        pthread_rwlock_unlock(&this_module->rx.rwlock);
        
        //发送
        pthread_rwlock_wrlock(&this_module->tx.rwlock);
        if(txset && MOD_ISSET(i, txset) && !MOD_ISSET(i, &this_module->tx.set)) {
            MOD_SET(i, &this_module->tx.set);
            pthread_rwlock_wrlock(&peer_module->rx.rwlock);
            MOD_SET(moduleID, &peer_module->rx.set);
            pthread_rwlock_wrlock(&peer_module->rx.rwlock);
        
            __fastq_create_ring( peer_module, moduleID, i);
        }
        pthread_rwlock_unlock(&this_module->tx.rwlock);
    }
    
    pthread_rwlock_unlock(&_AllModulesRingsLock);
    __atomic_store_n(&this_module->status, MODULE_STATUS_OK, __ATOMIC_RELEASE);

    return true;
}

always_inline bool inline
FastQDeleteModule(const char *name, const unsigned long moduleID, void *residual_msgs, int *residual_nbytes) {
    if(!name && (moduleID <= 0 || moduleID > FASTQ_ID_MAX) ) {
        fastq_log("Try to delete not exist MODULE.\n");
        assert(0);
        return false;
    }
    unsigned long _moduleID = moduleID;

    pthread_rwlock_wrlock(&_AllModulesRingsLock);
    
    //检查模块是否已经注册
    if(__atomic_load_n(&_AllModulesRings[_moduleID].already_register, __ATOMIC_RELAXED)) {
        LOG_DEBUG("Try delete Module %ld.\n", _moduleID);
        _fastq_fprintf(stderr, "\033[1;5;31m Delete Module ID %ld, Which register in file <%s>'s function <%s> at line %d\033[m\n", \
                        _moduleID,
                        _AllModulesRings[_moduleID]._file,
                        _AllModulesRings[_moduleID]._func,
                        _AllModulesRings[_moduleID]._line);
        
        pthread_rwlock_unlock(&_AllModulesRingsLock);
        assert(0);
        return ;
    }

    return true;
}


/**
 *  __FastQSend - 公共发送函数
 */
always_inline static bool inline
__FastQSend(struct FastQRing *ring, const void *msg, const size_t size) {
    assert(ring);
    assert(size <= (ring->_msg_size - sizeof(size_t)));

    unsigned int h = (ring->_head - 1) & ring->_size;
    unsigned int t = ring->_tail;
    if (t == h) {
        return false;
    }

    LOG_DEBUG("Send %ld->%ld.\n", ring->src, ring->dst);

    char *d = &ring->_ring_data[t*ring->_msg_size];
    
    memcpy(d, &size, sizeof(size));
    LOG_DEBUG("Send >>> memcpy msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, size);
    memcpy(d + sizeof(size), msg, size);
    LOG_DEBUG("Send >>> memcpy addr = %x\n", *(unsigned long*)(d + sizeof(size)));

    // Barrier is needed to make sure that item is updated 
    // before it's made available to the reader
    mwbarrier();
    
    //统计功能
    atomic64_inc(&ring->nr_enqueue);

    ring->_tail = (t + 1) & ring->_size;
    return true;
}

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
FastQSend(unsigned int from, unsigned int to, const void *msg, size_t size) {

    struct FastQRing *ring = __atomic_load_n(&_AllModulesRings[to]._ring[from], __ATOMIC_RELAXED);

    while (!__FastQSend(ring, msg, size)) {__relax();}
    
    eventfd_write(ring->_evt_fd, 1);
    
    LOG_DEBUG("Send done %ld->%ld, event fd = %d, msg = %p.\n", ring->src, ring->dst, ring->_evt_fd, msg);
    return true;
}

always_inline bool inline
FastQSendByName(const char* from, const char* to, const void *msg, size_t size) {

    assert(from && "NULL string.");
    assert(to && "NULL string.");
    
    unsigned long from_id = dict_find_module_id_byname(dictModuleNameID, from);
    unsigned long to_id = dict_find_module_id_byname(dictModuleNameID, to);
    
    if(unlikely(!__atomic_load_n(&_AllModulesRings[from_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    } if(unlikely(!__atomic_load_n(&_AllModulesRings[to_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    }
    return FastQSend(from_id, to_id, msg, size);
}



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
FastQTrySend(unsigned int from, unsigned int to, const void *msg, size_t size) {

    struct FastQRing *ring = __atomic_load_n(&_AllModulesRings[to]._ring[from], __ATOMIC_RELAXED);
    
    LOG_DEBUG("Send >>> msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, size);
    bool ret = __FastQSend(ring, msg, size);
    if(ret) {
        eventfd_write(ring->_evt_fd, 1);
        LOG_DEBUG("Send done %ld->%ld, event fd = %d.\n", ring->src, ring->dst, ring->_evt_fd);
    }
    return ret;
}

always_inline bool inline
FastQTrySendByName(const char* from, const char* to, const void *msg, size_t size) {

    assert(from && "NULL string.");
    assert(to && "NULL string.");
    unsigned long from_id = dict_find_module_id_byname(dictModuleNameID, from);
    unsigned long to_id = dict_find_module_id_byname(dictModuleNameID, to);
    if(unlikely(!__atomic_load_n(&_AllModulesRings[from_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    } if(unlikely(!__atomic_load_n(&_AllModulesRings[to_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    }

    return FastQTrySend(from_id, to_id, msg, size);
}

always_inline static bool inline
__FastQRecv(struct FastQRing *ring, void *msg, size_t *size) {

    unsigned int t = ring->_tail;
    unsigned int h = ring->_head;
    if (h == t) {
//        LOG_DEBUG("Recv <<< %ld->%ld.\n", ring->src, ring->dst);
        return false;
    }
    
    LOG_DEBUG("Recv <<< %ld->%ld.\n", ring->src, ring->dst);

    char *d = &ring->_ring_data[h*ring->_msg_size];

    size_t recv_size;
    memcpy(&recv_size, d, sizeof(size_t));
    LOG_DEBUG("Recv <<< memcpy recv_size = %d\n", recv_size);
    assert(recv_size <= *size && "buffer too small");
    *size = recv_size;
    LOG_DEBUG("Recv <<< size\n");
    memcpy(msg, d + sizeof(size_t), recv_size);
    LOG_DEBUG("Recv <<< memcpy addr = %x\n", *(unsigned long*)(d + sizeof(size_t)));
    LOG_DEBUG("Recv <<< memcpy msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, *size);

    // Barrier is needed to make sure that we finished reading the item
    // before moving the head
    mbarrier();
    LOG_DEBUG("Recv <<< mbarrier\n");
    //统计功能
    atomic64_inc(&ring->nr_dequeue);

    ring->_head = (h + 1) & ring->_size;
    return true;
}

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
FastQRecv(unsigned int from, fq_msg_handler_t handler) {

    assert(handler && "NULL pointer error.");

    eventfd_t cnt;
    int nfds;
#if defined(_FASTQ_EPOLL)
    struct epoll_event events[8];
#elif defined(_FASTQ_SELECT)
    int i, max_fd;
#endif 
    int curr_event_fd;
    char __attribute__((aligned(64))) addr[4096] = {0}; //page size
    size_t size = sizeof(addr);
    struct FastQRing *ring = NULL;

    while(1) {
#if defined(_FASTQ_EPOLL)
        LOG_DEBUG("Recv start >>>>  epoll fd = %d.\n", _AllModulesRings[from].epfd);
        nfds = epoll_wait(_AllModulesRings[from].epfd, events, 8, -1);
        LOG_DEBUG("Recv epoll return epfd = %d.\n", _AllModulesRings[from].epfd);
#elif defined(_FASTQ_SELECT)
        _AllModulesRings[from].selector.readset = _AllModulesRings[from].selector.allset;
        max_fd = _AllModulesRings[from].selector.maxfd;
        nfds = select(max_fd+1, &_AllModulesRings[from].selector.readset, NULL, NULL, NULL);
#endif 
        
#if defined(_FASTQ_EPOLL)
        for(;nfds--;) {
            curr_event_fd = events[nfds].data.fd;
#elif defined(_FASTQ_SELECT)
            
        for (i = 3; i <= max_fd; ++i) {
            if(!FD_ISSET(i, &_AllModulesRings[from].selector.readset)) {
                continue;
            }
            nfds--;
            curr_event_fd = i;
#endif 
            ring = __atomic_load_n(&_evtfd_to_ring[curr_event_fd].tlb_ring, __ATOMIC_RELAXED);
            eventfd_read(curr_event_fd, &cnt);

            LOG_DEBUG("Event fd %d read return cnt = %ld.\n", curr_event_fd, cnt);
            for(; cnt--;) {
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv).\n");
                while (!__FastQRecv(ring, addr, &size)) {
                    __relax();
                }
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv) addr = %lx, size = %ld.\n", *(unsigned long*)addr, size);
                handler((void*)addr, size);
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv) done.\n");
            }

        }
    }
    return true;
}

always_inline  bool inline
FastQRecvByName(const char *from, fq_msg_handler_t handler) { 
    assert(from && "NULL string.");
    unsigned long from_id = dict_find_module_id_byname(dictModuleNameID, from);
    if(unlikely(!__atomic_load_n(&_AllModulesRings[from_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    }
    return FastQRecv(from_id, handler);
}


/**
 *  FastQInfo - 查询信息
 *  
 *  param[in]   fp    文件指针,当 fp == NULL，默认使用 stderr 
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline bool inline
FastQMsgStatInfo(struct FastQModuleMsgStatInfo *buf, unsigned int buf_mod_size, unsigned int *num, 
                            fq_module_filter_t filter) {
    
    assert(buf && num && "NULL pointer error.");
    assert(buf_mod_size && "buf_mod_size MUST bigger than zero.");

    unsigned long dstID, srcID, bufIdx = 0;
    *num = 0;

    for(dstID=1; dstID<=FASTQ_ID_MAX; dstID++) {
//        printf("------------------ %d -> reg %d\n",dstID, _AllModulesRings[dstID].already_register);
        if(!__atomic_load_n(&_AllModulesRings[dstID].already_register, __ATOMIC_RELAXED)) {
            continue;
        }
        if(!__atomic_load_n(&_AllModulesRings[dstID]._ring, __ATOMIC_RELAXED)) {
            continue;
        }
        for(srcID=0; srcID<=FASTQ_ID_MAX; srcID++) { 
            
            if(!__atomic_load_n(&_AllModulesRings[dstID]._ring[srcID], __ATOMIC_RELAXED)) {
                continue;
            }
        
            //过滤掉一些
            if(filter) {
                if(!filter(srcID, dstID)) continue;
            }
            buf[bufIdx].src_module = srcID;
            buf[bufIdx].dst_module = dstID;

            LOG_DEBUG("enqueue = %ld\n", atomic64_read(&_AllModulesRings[dstID]._ring[srcID]->nr_enqueue));
            LOG_DEBUG("dequeue = %ld\n", atomic64_read(&_AllModulesRings[dstID]._ring[srcID]->nr_dequeue));
            
            buf[bufIdx].enqueue = atomic64_read(&_AllModulesRings[dstID]._ring[srcID]->nr_enqueue);
            buf[bufIdx].dequeue = atomic64_read(&_AllModulesRings[dstID]._ring[srcID]->nr_dequeue);
            
            bufIdx++;
            (*num)++;
            if(buf_mod_size == bufIdx) 
                return true;
        }
    }
    return true;
}

/**
 *  FastQDump - 显示信息
 *  
 *  param[in]   fp    文件指针,当 fp == NULL，默认使用 stderr 
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline void inline
FastQDump(FILE*fp, unsigned long module_id) {
    
    if(unlikely(!fp)) {
        fp = stderr;
    }
    _fastq_fprintf(fp, "\n FastQ Dump Information.\n");

    unsigned long i, j, max_module = FASTQ_ID_MAX;

    if(module_id == 0 || module_id > FASTQ_ID_MAX) {
        i = 1;
        max_module = FASTQ_ID_MAX;
    } else {
        i = module_id;
        max_module = module_id;
    }
    
    
    for(i=1; i<=max_module; i++) {
        if(!__atomic_load_n(&_AllModulesRings[i].already_register, __ATOMIC_RELAXED)) {
            continue;
        }
        _fastq_fprintf(fp, "\033[1;31mModule ID %ld register in file <%s>'s function <%s> at line %d\033[m\n", \
                        i,
                        _AllModulesRings[i]._file,
                        _AllModulesRings[i]._func,
                        _AllModulesRings[i]._line);
        atomic64_t module_total_msgs[2];
        atomic64_init(&module_total_msgs[0]); //总入队数量
        atomic64_init(&module_total_msgs[1]); //总出队数量
        _fastq_fprintf(fp, "------------------------------------------\n"\
                    "ID: %3ld, msgMax %4u, msgSize %4u\n"\
                    "\t from-> to   %10s %10s"
                    " %16s %16s %16s "
                    "\n"
                    , i, 
                    _AllModulesRings[i].ring_size, 
                    _AllModulesRings[i].msg_size, 
                    "head", 
                    "tail"
                    , "enqueue", "dequeue", "current"
                    );
        
        for(j=0; j<=FASTQ_ID_MAX; j++) { 
            if(__atomic_load_n(&_AllModulesRings[i]._ring[j], __ATOMIC_RELAXED)) {
                _fastq_fprintf(fp, "\t %4ld->%-4ld  %10u %10u"
                            " %16ld %16ld %16ld"
                            "\n" \
                            , j, i, 
                            _AllModulesRings[i]._ring[j]->_head, 
                            _AllModulesRings[i]._ring[j]->_tail
                            ,atomic64_read(&_AllModulesRings[i]._ring[j]->nr_enqueue),
                            atomic64_read(&_AllModulesRings[i]._ring[j]->nr_dequeue),
                            atomic64_read(&_AllModulesRings[i]._ring[j]->nr_enqueue)
                                -atomic64_read(&_AllModulesRings[i]._ring[j]->nr_dequeue)
                            );
                atomic64_add(&module_total_msgs[0], atomic64_read(&_AllModulesRings[i]._ring[j]->nr_enqueue));
                atomic64_add(&module_total_msgs[1], atomic64_read(&_AllModulesRings[i]._ring[j]->nr_dequeue));
            }
        }
        
        _fastq_fprintf(fp, "\t Total enqueue %16ld, dequeue %16ld\n", atomic64_read(&module_total_msgs[0]), atomic64_read(&module_total_msgs[1]));
    }
    fflush(fp);
    return;
}


always_inline  bool inline
FastQMsgNum(unsigned int ID, 
            unsigned long *nr_enqueues, unsigned long *nr_dequeues, unsigned long *nr_currents) {

    if(ID <= 0 || ID > FASTQ_ID_MAX) {
        return false;
    }
    if(!__atomic_load_n(&_AllModulesRings[ID].already_register, __ATOMIC_RELAXED)) {
        return false;
    }
//    printf("FastQMsgNum .\n");
    
    int i;
    *nr_dequeues = *nr_enqueues = *nr_currents = 0;
    
    for(i=1; i<=FASTQ_ID_MAX; i++) {
        if(MOD_ISSET(i, &_AllModulesRings[ID].rx.set)) {
            *nr_enqueues += atomic64_read(&_AllModulesRings[ID]._ring[i]->nr_enqueue);
            *nr_dequeues += atomic64_read(&_AllModulesRings[ID]._ring[i]->nr_dequeue);
        }
    }
    *nr_currents = (*nr_enqueues) - (*nr_dequeues);
    
    return true;

}

weak_alias(FastQCreateModule, FastQCreateModuleStats);
weak_alias(FastQDeleteModule, FastQDeleteModuleStats);
weak_alias(FastQAddSet, FastQAddSetStats);
weak_alias(FastQDump, FastQDumpStats);
weak_alias(FastQMsgStatInfo, FastQMsgStatInfoStats);
weak_alias(FastQSend, FastQSendStats);
weak_alias(FastQSendByName, FastQSendByNameStats);
weak_alias(FastQTrySend, FastQTrySendStats);
weak_alias(FastQTrySendByName, FastQTrySendByNameStats);
weak_alias(FastQRecv, FastQRecvStats);
weak_alias(FastQRecvByName, FastQRecvByNameStats);
weak_alias(FastQMsgNum, FastQMsgNumStats);

#pragma GCC diagnostic pop

