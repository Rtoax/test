/********************************************************************\
*  文件： nanomq.h
*  介绍： 低时延队列
*  作者： 荣涛
*  日期：
*       2021年1月25日    
\********************************************************************/

#ifndef __nAnoMQ_H
#define __nAnoMQ_H 1

#include <stdbool.h>



// Forced inlining 
#ifndef always_inline
#define always_inline __attribute__ ((__always_inline__))
#endif

#ifdef always_inline
#undef always_inline 
#define always_inline 
#endif


// POD for header data
struct nmq_header;

// POD for nmq_ring
struct nmq_ring;

/**
 *  队列上下文
 */
typedef struct nmq_context {
#define CTX_FNAME_LEN   512
    void *p_;
    struct nmq_header *header_;
    struct nmq_ring *ring_;
    char *data_;
};







always_inline bool nmq_ctx_create(struct nmq_context *self, unsigned int nodes, unsigned int size, unsigned int msg_size);
always_inline void nmq_ctx_print(struct nmq_context *self);

always_inline bool nmq_ctx_sendto(struct nmq_context *self, unsigned int from, unsigned int to, const void *msg, size_t size);
always_inline bool nmq_ctx_sendnb(struct nmq_context *self, unsigned int from, unsigned int to, const void *msg, size_t size);
always_inline bool nmq_ctx_recvfrom(struct nmq_context *self, unsigned int from, unsigned int to, void *msg, size_t *size);
always_inline bool nmq_ctx_recvnb(struct nmq_context *self, unsigned int from, unsigned int to, void *s, size_t *size);



#endif /*<__nAnoMQ_H>*/
