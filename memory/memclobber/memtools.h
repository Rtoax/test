#ifndef __HPCOMMON_H
#define __HPCOMMON_H 1

#define LIBC_SO     "libc.so.6"//"/usr/lib64/libc.so.6"


typedef void *(*malloc_fn_t)(size_t size);
typedef void  (*free_fn_t)(void *ptr);
typedef void *(*calloc_fn_t)(size_t nmemb, size_t size);
typedef void *(*realloc_fn_t)(void *ptr, size_t size);
typedef char *(*strdup_fn_t)(const char *s);
typedef char *(*strndup_fn_t)(const char *s, size_t n);

typedef void *(*__canhook__malloc_fn_t)(unsigned long idx, size_t size, const char* _func, const int _line);
typedef void (*__canhook__free_fn_t)(unsigned long idx, void *ptr, const char* _func, const int _line);
typedef void *(*__canhook__calloc_fn_t)(unsigned long idx, size_t nmemb, size_t size, const char* _func, const int _line);
typedef void *(*__canhook__realloc_fn_t)(unsigned long idx, void *ptr, size_t size, const char* _func, const int _line);
typedef char *(*__canhook__strdup_fn_t)(unsigned long idx, const char *s, const char* _func, const int _line);
typedef char *(*__canhook__strndup_fn_t)(unsigned long idx, const char *s, size_t n, const char* _func, const int _line);


/*--------------------------------------------------------------------------------------------------------------------*\
 *      宏定义
\*--------------------------------------------------------------------------------------------------------------------*/
#ifndef unlikely
#define unlikely(x)    (__builtin_expect(!!(x), 0))
#endif

#ifndef likely
#define likely(x)    (__builtin_expect(!!(x), 1))
#endif

#ifndef WRITE_ONCE
#define WRITE_ONCE(var, val) (*((volatile typeof(val) *)(&(var))) = (val))
#endif

#ifndef READ_ONCE
#define READ_ONCE(var) (*((volatile typeof(var) *)(&(var))))
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif 

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#ifndef container_of
#define container_of(ptr, type, member) ({          \
	const typeof(((type *)0)->member)*__mptr = (ptr);    \
		     (type *)((char *)__mptr - offsetof(type, member)); })
#endif

#ifndef weak_alias
#define weak_alias(name, aliasname) \
            extern typeof (name) aliasname __attribute__ ((weak, alias(#name)))
#endif

/*--------------------------------------------------------------------------------------------------------------------*\
 *  使用原子操作，规避同步锁性能问题
\*--------------------------------------------------------------------------------------------------------------------*/
typedef size_t Atomic_t;

/* To test Redis with Helgrind (a Valgrind tool) it is useful to define
 * the following macro, so that __sync macros are used: those can be detected
 * by Helgrind (even if they are less efficient) so that no false positive
 * is reported. */
// #define __ATOMIC_VAR_FORCE_SYNC_MACROS

#if !defined(__ATOMIC_VAR_FORCE_SYNC_MACROS) && defined(__ATOMIC_RELAXED) 
/* Implementation using __atomic macros. "atomic-builtin" */

#define atomic_incr(var,count) __atomic_add_fetch(&var,(count),__ATOMIC_RELAXED)
#define atomic_get_incr(var,oldvalue_var,count) do { \
    oldvalue_var = __atomic_fetch_add(&var,(count),__ATOMIC_RELAXED); \
} while(0)
#define atomic_decr(var,count) __atomic_sub_fetch(&var,(count),__ATOMIC_RELAXED)
#define atomic_get(var,dstvar) do { \
    dstvar = __atomic_load_n(&var,__ATOMIC_RELAXED); \
} while(0)
#define atomic_set(var,value) __atomic_store_n(&var,value,__ATOMIC_RELAXED)

#elif defined(HAVE_ATOMIC)
/* Implementation using __sync macros. "sync-builtin" */

#define atomic_incr(var,count) __sync_add_and_fetch(&var,(count))
#define atomic_get_incr(var,oldvalue_var,count) do { \
    oldvalue_var = __sync_fetch_and_add(&var,(count)); \
} while(0)
#define atomic_decr(var,count) __sync_sub_and_fetch(&var,(count))
#define atomic_get(var,dstvar) do { \
    dstvar = __sync_sub_and_fetch(&var,0); \
} while(0)
#define atomic_set(var,value) do { \
    while(!__sync_bool_compare_and_swap(&var,var,value)); \
} while(0)

#endif


#endif /*<__HPCOMMON_H>*/

