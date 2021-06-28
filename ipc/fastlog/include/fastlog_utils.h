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




typedef struct {
    volatile int64_t cnt;
} fastlog_atomic64_t;



inline int  
fastlog_atomic64_cmpset(volatile uint64_t *dst, uint64_t exp, uint64_t src);
inline void 
fastlog_atomic64_init(fastlog_atomic64_t *v);
inline int64_t 
fastlog_atomic64_read(fastlog_atomic64_t *v);
inline void 
fastlog_atomic64_add(fastlog_atomic64_t *v, int64_t inc);
inline void 
fastlog_atomic64_inc(fastlog_atomic64_t *v);
inline void
fastlog_atomic64_dec(fastlog_atomic64_t *v);


inline int 
__fastlog_sched_getcpu();
inline unsigned 
__fastlog_getcpu();
inline  uint64_t 
__fastlog_rdtsc();





#endif /*<__fAStLOG_UTILS_H>*/
