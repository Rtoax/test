#ifndef __fAStLOG_UTILS_H
#define __fAStLOG_UTILS_H 1

#ifndef __fAStLOG_H
#error "You can't include <fastlog_utils.h> directly, #include <fastlog.h> instead."
#endif

#include <stdint.h>

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
#define __cachelinealigned __attribute__((aligned(64)))
#define _unused             __attribute__((unused))


/**
 *  `7`: 并不一定是`7`
 *
 *  意在消除 编译警告
 *  warning: inline function ‘__fastlog_sched_getcpu’ declared but never defined
 */
#if defined (__GNUC__) && (__GNUC__ >= 7)
#define fl_inline 
#else
#define fl_inline   inline
#endif



typedef struct {
    volatile int64_t cnt;
} fastlog_atomic64_t;



fl_inline int  
fastlog_atomic64_cmpset(volatile uint64_t *dst, uint64_t exp, uint64_t src);
fl_inline void 
fastlog_atomic64_init(fastlog_atomic64_t *v);
fl_inline int64_t 
fastlog_atomic64_read(fastlog_atomic64_t *v);
fl_inline void 
fastlog_atomic64_add(fastlog_atomic64_t *v, int64_t inc);
fl_inline void 
fastlog_atomic64_inc(fastlog_atomic64_t *v);
fl_inline void
fastlog_atomic64_dec(fastlog_atomic64_t *v);


fl_inline int 
__fastlog_sched_getcpu();
fl_inline unsigned 
__fastlog_getcpu();
fl_inline  uint64_t 
__fastlog_rdtsc();





#endif /*<__fAStLOG_UTILS_H>*/
