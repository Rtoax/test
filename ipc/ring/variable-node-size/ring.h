/**
 *  可变节点大小的队列
 *  
 *  作者：荣涛
 *  日期：
 *      2021年6月1日 创建
 *  
 */
#ifndef ____RING_H
#define ____RING_H 1

#include <assert.h>
#include <malloc.h>
#include <string.h>
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
//    int _prev_data_size;
    int _data_size;
    char _data[];
};

struct __ring {
    unsigned int _nodes_size;   //总大小
//    unsigned int _nodes_size_remaining;    //剩下的
    
    volatile unsigned int  _head;
    volatile unsigned int  _tail;

    char _nodes[];
} __cachelinealigned;





static unsigned int _unused __power_of_2(unsigned int size) 
{
    unsigned int i;
    for (i=0; (1U << i) < size; i++);
    return 1U << i;
}


static struct __ring* _unused __ring_create(size_t size)
{
    unsigned int nodes_size = __power_of_2(size);
    unsigned int total_size = nodes_size + sizeof(struct __ring);

    printf("nodes_size = %d, total_size = %d\n", nodes_size, total_size);

    struct __ring *new_ring = malloc(total_size);
    
    assert(new_ring && "OOM error");

    memset(new_ring, 0x00, total_size);
    
    new_ring->_nodes_size = nodes_size;
//    new_ring->_nodes_size_remaining = nodes_size;
    new_ring->_head = 0;
    new_ring->_tail = 0;


    return new_ring;
}


static inline bool  __ring_enqueue(struct __ring *ring, const void *msg, const size_t size) 
{
    assert(ring);
    assert(msg);
    
    const size_t node_size = size + sizeof(struct __ring_node);

    unsigned int h = ring->_head;
//    bool reverse = (ring->_tail + size) > ring->_nodes_size ? true:false;
    unsigned int curr_t = ring->_tail;
    unsigned int t = (curr_t + size) & (ring->_nodes_size-1);
    
//    printf("h = %d, t = %d\n", h, t);
    
    if (t - h < size) {
        printf("full.\n");
        return false;
    }

    struct __ring_node *node = (struct __ring_node *)&ring->_nodes[curr_t];

    node->_data_size  = size;
    memcpy(node->_data, msg, size);
    
    mwbarrier();
    
    printf("insert: head = %d, tail = %d, size = %ld\n", ring->_head, curr_t, node->_data_size);
    
    ring->_tail = (curr_t + node_size) & (ring->_nodes_size-1);
    
    return true;
}


static bool inline __ring_dequeue(struct __ring *ring, void *msg, size_t *size) 
{
    assert(ring);
    assert(msg);
    assert(size);
    
    unsigned int t = ring->_tail;
    unsigned int h = ring->_head;
    
//    printf("h = %d, t = %d\n", h, t);
    
    if (h == t) {
        printf("empty.\n");
        return false;
    }

    struct __ring_node *node = (struct __ring_node *)&ring->_nodes[h];

    *size = node->_data_size;
    
    const size_t node_size = *size + sizeof(struct __ring_node);
    
    memcpy(msg, node->_data, node->_data_size);

    mbarrier();
    
    printf("delete: head = %d, tail = %d, size = %ld(%ld)\n", ring->_head, ring->_tail, node->_data_size, *size);
    
    ring->_head = (h + node_size) & (ring->_nodes_size-1);
    return true;
}



#endif /*<____RING_H>*/
