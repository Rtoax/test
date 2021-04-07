#ifndef __fAStMQ_H
#error  Not include fastq_types.h directly, include fastq.h instead.
#endif


#include <stdbool.h>


#ifndef always_inline
#define always_inline /*__attribute__ ((__always_inline__))*/
#endif


#ifndef __fAStMQ_TYpE_H
#define __fAStMQ_TYpE_H 1
/**
 *  Crypto
 */
#define __MOD_SETSIZE  FASTQ_ID_MAX
#define __NMOD     (8 * (int) sizeof (__mod_mask))
#define __MOD_ELT(d)   ((d) / __NMOD)
#define __MOD_MASK(d)  ((__mod_mask)(1UL << ((d) % __NMOD)))

typedef long int __mod_mask;
typedef struct {
    __mod_mask __mod[__MOD_SETSIZE/__NMOD];
#define __MOD(set) ((set)->__mod)
#define __MOD_SET_INITIALIZER    {0}
}__attribute__((aligned(64))) __mod_set;


#define __MOD_ZERO(s) \
    do {    \
        unsigned int __i;   \
        __mod_set *__arr = (s);  \
        for (__i = 0; __i < sizeof (__mod_set) / sizeof (__mod_mask); ++__i)    \
            __MOD (__arr)[__i] = 0;  \
    } while (0)
#define __MOD_SET(d, s) \
    ((void) (__MOD (s)[__MOD_ELT(d)] |= __MOD_MASK(d)))
#define __MOD_CLR(d, s) \
    ((void) (__MOD (s)[__MOD_ELT(d)] &= ~ __MOD_MASK(d)))
#define __MOD_ISSET(d, s) \
    ((__MOD (s)[__MOD_ELT (d)] & __MOD_MASK (d)) != 0)
    

#endif /*<__fAStMQ_TYpE_H>*/
