/**
 *  可变节点大小的队列
 *  
 *  作者：荣涛
 *  日期：
 *      2021年6月1日 创建 并完成初始版本
 *      2021年6月4日 添加自旋锁保护 _head _tail
 *  
 */
#ifndef ____RING_H
#define ____RING_H 1

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#ifndef likely
#define likely(x)    __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#ifndef _unused
#define _unused             __attribute__((unused))
#endif

//#define RING_DEBUG

#ifdef RING_DEBUG
#define DEBUG_LOG(fmt...) do{\
        fprintf(stderr, "[%s:%d] ", __func__, __LINE__);\
        printf(fmt);    \
    }while(0)
#define inline
#else
#define DEBUG_LOG(fmt...) do{}while(0)
#endif

static void  inline _unused mbarrier() { asm volatile("": : :"memory"); }
static void  inline _unused mrwbarrier() { asm volatile("mfence":::"memory"); }
static void  inline _unused mrbarrier()  { asm volatile("lfence":::"memory"); }
static void  inline _unused mwbarrier()  { asm volatile("sfence":::"memory"); }
static void  inline _unused __relax()  { asm volatile ("pause":::"memory"); }


struct __ring_node {
    unsigned int _valide:1;
    unsigned int _reserve:7;
    unsigned int _data_size:8;  //最大数据大小 256
    char _data[];
}__attribute__((packed));

struct __ring {
    size_t _nodes_size;   //总大小
//    pthread_spinlock_t spinlock;

    char _pad1[64];
    volatile int  _head;
    char _pad2[64];
    volatile int  _tail;
    char _pad3[64];

    char _nodes[];
} __attribute__((aligned(64)));



static size_t inline _unused __power_of_2(unsigned int size) 
{
    unsigned int i;
    for (i=0; (1U << i) < size; i++);
    return 1U << i;
}


static struct __ring* _unused __ring_create(size_t size)
{
    size_t nodes_size = __power_of_2(size);
    size_t total_size = nodes_size + sizeof(struct __ring);

    struct __ring *new_ring = (struct __ring *)malloc(total_size);
    
    assert(new_ring && "OOM error");

    memset(new_ring, 0x00, total_size);
    
    new_ring->_nodes_size = nodes_size;
    new_ring->_head = 0;
    new_ring->_tail = 0;

//    pthread_spin_init(&new_ring->spinlock, PTHREAD_PROCESS_PRIVATE);

    return new_ring;
}


/*
----- 空闲
##### 已使用
***** 即将填充
%%%%% 空闲但不使用
*/
static inline bool _unused __ring_enqueue(struct __ring *ring, const void *msg, const size_t size) 
{
    assert(ring);
    assert(msg);
    assert(size < ring->_nodes_size);

//    pthread_spin_lock(&ring->spinlock);
    
    const size_t node_size = size + sizeof(struct __ring_node);

    size_t head = ring->_head;
    size_t tail = ring->_tail;
    size_t next_tail = (tail + node_size) & (ring->_nodes_size-1);

    /* tail指针将翻转
        
         next_tail   head    tail  
            **-------##########%%%%%%%%%%%
     */
    bool beyond = (next_tail < tail);
    if(unlikely(beyond)) {
        DEBUG_LOG("beyond.\n");
        struct __ring_node *tmp = (struct __ring_node *)&ring->_nodes[tail];

        mwbarrier();
        tmp->_data_size = ring->_nodes_size - tail;
        tmp->_valide    = 0;
        tail = 0;
        next_tail = node_size;
        if(next_tail >= head) {
            DEBUG_LOG("full1. (%d,%d)\n", ring->_head, ring->_tail);
//            pthread_spin_unlock(&ring->spinlock);
            return false;
        }
    } else {
        /*
                    head    tail  next_tail
            ---------##########*****------
         */
        if(ring->_nodes_size - tail < node_size) {
            DEBUG_LOG("full3. (%d,%d)\n", ring->_head, ring->_tail);
//            pthread_spin_unlock(&ring->spinlock);
            return false;
        }
        /*
             tail  head      
            ###***---#################%%%%
               next_tail
         */
        if(tail < head && next_tail > head) {
            DEBUG_LOG("full4. (%d,%d)\n", ring->_head, ring->_tail);
//            pthread_spin_unlock(&ring->spinlock);
            return false;
        }
    }
    
    struct __ring_node *node = (struct __ring_node *)&ring->_nodes[tail];

    node->_data_size    = size;
    node->_valide       = 1;
    memcpy(node->_data, msg, size);
    
    mwbarrier();
    
    DEBUG_LOG("insert: head = %d, tail = %d, size = %ld, node = %p\n", ring->_head, tail, node->_data_size, node);
    
    ring->_tail = tail + node_size;
    
//    pthread_spin_unlock(&ring->spinlock);
    return true;
}


static bool inline _unused __ring_dequeue( struct __ring *const ring, void *msg, size_t *size) 
{
    assert(ring);
    assert(msg);
    assert(size);
    
//    pthread_spin_lock(&ring->spinlock);

    size_t tail = ring->_tail;
    size_t head = ring->_head;

try_again:

    
    if (head == tail) {
        DEBUG_LOG("empty.\n");
//        pthread_spin_unlock(&ring->spinlock);
        return false;
    }

    struct __ring_node *node = (struct __ring_node *)&ring->_nodes[head];
    
    /* 结尾的 不可用包 %%%
       
        next_tail   head    tail  
           xxxxxxxxxxxxxxxxxxxxxxxx%%%
    */
    if(!node->_valide) {
        DEBUG_LOG("invalide node.\n");
        mbarrier();
        ring->_head = 0;
        head = 0;
        goto try_again;
    }
    
    *size = node->_data_size;
        
    const size_t node_size = (*size) + sizeof(struct __ring_node);

    memcpy(msg, node->_data, *size);

    bool beyond = !!((head + node_size) > ring->_nodes_size);

    mbarrier();
    ring->_head = beyond?0:(head + node_size) & (ring->_nodes_size-1);
    
    DEBUG_LOG("delete: head = %d, tail = %d, size = %ld, node = %p\n", ring->_head, tail, node->_data_size, node);
    
//    pthread_spin_unlock(&ring->spinlock);
    return true;
}



#endif /*<____RING_H>*/
