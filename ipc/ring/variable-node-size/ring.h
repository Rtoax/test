/**
 *  可变节点大小的队列
 *  
 *  作者：荣涛
 *  日期：
 *      2021年6月1日 创建 并完成初始版本
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
#ifndef __cachelinealigned
#define __cachelinealigned __attribute__((aligned(64)))
#endif
#ifndef _unused
#define _unused             __attribute__((unused))
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
    
    volatile size_t  _head;
    volatile size_t  _tail;

    char _nodes[];
} __cachelinealigned;


static size_t _unused __power_of_2(unsigned int size) 
{
    unsigned int i;
    for (i=0; (1U << i) < size; i++);
    return 1U << i;
}


static struct __ring* _unused __ring_create(size_t size)
{
    size_t nodes_size = __power_of_2(size);
    size_t total_size = nodes_size + sizeof(struct __ring);

    printf("nodes_size = %d, total_size = %d\n", nodes_size, total_size);

    struct __ring *new_ring = (struct __ring *)malloc(total_size);
    
    assert(new_ring && "OOM error");

    memset(new_ring, 0x00, total_size);
    
    new_ring->_nodes_size = nodes_size;
    new_ring->_head = size/2;
    new_ring->_tail = size/2;


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
//        printf("beyond.\n");
        struct __ring_node *tmp = (struct __ring_node *)&ring->_nodes[tail];
        tmp->_data_size = ring->_nodes_size - tail;
        tmp->_valide    = 0;
        tail = 0;
        next_tail = node_size;
        if(next_tail >= head) {
//            printf("full1. (%d,%d)\n", ring->_head, ring->_tail);
            return false;
        }
    } else {
        /*
                    head    tail  next_tail
            ---------##########*****------
         */
        if(ring->_nodes_size - tail < node_size) {
//            printf("full3. (%d,%d)\n", ring->_head, ring->_tail);
            return false;
        }
        /*
             tail  head      
            ###***---#################%%%%
               next_tail
         */
        if(tail < head && next_tail > head) {
//            printf("full4. (%d,%d)\n", ring->_head, ring->_tail);
            return false;
        }
    }
    
    struct __ring_node *node = (struct __ring_node *)&ring->_nodes[tail];

    node->_data_size    = size;
    node->_valide       = 1;
    memcpy(node->_data, msg, size);
    
    mwbarrier();
    
//    printf("insert: head = %d, tail = %d, size = %ld, node = %p\n", ring->_head, tail, node->_data_size, node);
    
    ring->_tail = tail + node_size;
    
    return true;
}


static bool inline _unused __ring_dequeue( struct __ring *const ring, void *msg, size_t *size) 
{
    assert(ring);
    assert(msg);
    assert(size);

    size_t tail = ring->_tail;
    size_t head = ring->_head;

try_again:

    
    if (head == tail) {
//        printf("empty.\n");
        return false;
    }

    struct __ring_node *node = (struct __ring_node *)&ring->_nodes[head];
    
    /* 结尾的 不可用包 %%%
       
        next_tail   head    tail  
           xxxxxxxxxxxxxxxxxxxxxxxx%%%
    */
    if(!node->_valide) {
//        printf("invalide node.\n");
        mbarrier();
        ring->_head = 0;
        head = 0;
        goto try_again;
    }
    
    *size = node->_data_size;
    
    const size_t node_size = (*size) + sizeof(struct __ring_node);

    memcpy(msg, node->_data, *size);

    mbarrier();
    

    bool beyond = !!((head + node_size) > ring->_nodes_size);

    ring->_head = beyond?0:(head + node_size) & (ring->_nodes_size-1);
    
//    printf("delete: head = %d, tail = %d, size = %ld, node = %p\n", ring->_head, tail, node->_data_size, node);
    
    return true;
}



#endif /*<____RING_H>*/
