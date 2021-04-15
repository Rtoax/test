/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2019 Intel Corporation
 */

#ifndef _RTE_COMMON_H_
#define _RTE_COMMON_H_


#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

/** C extension macro for environments lacking C11 features. */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#define RTE_STD_C11 __extension__
#else
#define RTE_STD_C11
#endif

/* Maximum length of file name */
#if !defined(PATH_MAX)
#   define PATH_MAX 256
#endif

#define MSEC_PER_SEC           1000      /**< Milli-seconds per second */
#define USEC_PER_MSEC          1000      /**< Micro-seconds per milli-second */
#define USEC_PER_SEC           1000000   /**< Micro-seconds per second */
#define NSEC_PER_SEC           (USEC_PER_SEC * 1000) /**< Nano-seconds per second */

/**
 * Force a structure to be packed
 */
#define __rte_packed __attribute__((__packed__))

/******* Macro to mark functions and fields scheduled for removal *****/
#define __rte_deprecated	__attribute__((__deprecated__))

/**
 * Mark a function or variable to a weak reference.
 */
#define __rte_weak __attribute__((__weak__))

/**
 * Force symbol to be generated even if it appears to be unused.
 */
#define __rte_used __attribute__((used))

/*********** Macros to eliminate unused variable warnings ********/

/**
 * short definition to mark a function parameter unused
 */
#define __rte_unused __attribute__((__unused__))

/**
 * definition to mark a variable or function parameter as used so
 * as to avoid a compiler warning
 */
#define RTE_SET_USED(x) (void)(x)



/**
 * Check format string and its arguments at compile-time.
 *
 * GCC on Windows assumes MS-specific format string by default,
 * even if the underlying stdio implementation is ANSI-compliant,
 * so this must be overridden.
 */
#if RTE_CC_IS_GNU
#define __rte_format_printf(format_index, first_arg) \
	__attribute__((format(gnu_printf, format_index, first_arg)))
#else
#define __rte_format_printf(format_index, first_arg) \
	__attribute__((format(printf, format_index, first_arg)))
#endif

#define RTE_PRIORITY_LOG 101
#define RTE_PRIORITY_BUS 110
#define RTE_PRIORITY_CLASS 120
#define RTE_PRIORITY_LAST 65535

#define RTE_PRIO(prio) \
	RTE_PRIORITY_ ## prio

/**
 * Run function before main() with high priority.
 *
 * @param func
 *   Constructor function.
 * @param prio
 *   Priority number must be above 100.
 *   Lowest number is the first to run.
 */
#ifndef RTE_INIT_PRIO /* Allow to override from EAL */
#define RTE_INIT_PRIO(func, prio) \
static void __attribute__((constructor(RTE_PRIO(prio)), used)) func(void)
#endif

/**
 * Run function before main() with low priority.
 *
 * The constructor will be run after prioritized constructors.
 *
 * @param func
 *   Constructor function.
 */
#define RTE_INIT(func) \
	RTE_INIT_PRIO(func, LAST)

/**
 * Run after main() with low priority.
 *
 * @param func
 *   Destructor function name.
 * @param prio
 *   Priority number must be above 100.
 *   Lowest number is the last to run.
 */
#ifndef RTE_FINI_PRIO /* Allow to override from EAL */
#define RTE_FINI_PRIO(func, prio) \
static void __attribute__((destructor(RTE_PRIO(prio)), used)) func(void)
#endif

/**
 * Run after main() with high priority.
 *
 * The destructor will be run *before* prioritized destructors.
 *
 * @param func
 *   Destructor function name.
 */
#define RTE_FINI(func) \
	RTE_FINI_PRIO(func, LAST)

/**
 * Hint never returning function
 */
#define __rte_noreturn __attribute__((noreturn))

/**
 * Force a function to be inlined
 */
#define __rte_always_inline inline __attribute__((always_inline))

/**
 * Force a function to be noinlined
 */
#define __rte_noinline __attribute__((noinline))

/**
 * Hint function in the hot path
 */
#define __rte_hot __attribute__((hot))

/**
 * Hint function in the cold path
 */
#define __rte_cold __attribute__((cold))

/*********** Macros for pointer arithmetic ********/

/**
 * add a byte-value offset to a pointer
 */
#define RTE_PTR_ADD(ptr, x) ((void*)((uintptr_t)(ptr) + (x)))

/**
 * subtract a byte-value offset from a pointer
 */
#define RTE_PTR_SUB(ptr, x) ((void*)((uintptr_t)ptr - (x)))

/**
 * get the difference between two pointer values, i.e. how far apart
 * in bytes are the locations they point two. It is assumed that
 * ptr1 is greater than ptr2.
 */
#define RTE_PTR_DIFF(ptr1, ptr2) ((uintptr_t)(ptr1) - (uintptr_t)(ptr2))

/**
 * Workaround to cast a const field of a structure to non-const type.
 */
#define RTE_CAST_FIELD(var, field, type) \
	(*(type *)((uintptr_t)(var) + offsetof(typeof(*(var)), field)))

/*********** Macros/static functions for doing alignment ********/


/**
 * Macro to align a pointer to a given power-of-two. The resultant
 * pointer will be a pointer of the same type as the first parameter, and
 * point to an address no higher than the first parameter. Second parameter
 * must be a power-of-two value.
 */
#define RTE_PTR_ALIGN_FLOOR(ptr, align) \
	((typeof(ptr))RTE_ALIGN_FLOOR((uintptr_t)ptr, align))

/**
 * Macro to align a value to a given power-of-two. The resultant value
 * will be of the same type as the first parameter, and will be no
 * bigger than the first parameter. Second parameter must be a
 * power-of-two value.
 */
#define RTE_ALIGN_FLOOR(val, align) \
	(typeof(val))((val) & (~((typeof(val))((align) - 1))))

/**
 * Macro to align a pointer to a given power-of-two. The resultant
 * pointer will be a pointer of the same type as the first parameter, and
 * point to an address no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 */
#define RTE_PTR_ALIGN_CEIL(ptr, align) \
	RTE_PTR_ALIGN_FLOOR((typeof(ptr))RTE_PTR_ADD(ptr, (align) - 1), align)

/**
 * Macro to align a value to a given power-of-two. The resultant value
 * will be of the same type as the first parameter, and will be no lower
 * than the first parameter. Second parameter must be a power-of-two
 * value.
 */
#define RTE_ALIGN_CEIL(val, align) \
	RTE_ALIGN_FLOOR(((val) + ((typeof(val)) (align) - 1)), align)

/**
 * Macro to align a pointer to a given power-of-two. The resultant
 * pointer will be a pointer of the same type as the first parameter, and
 * point to an address no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 * This function is the same as RTE_PTR_ALIGN_CEIL
 */
#define RTE_PTR_ALIGN(ptr, align) RTE_PTR_ALIGN_CEIL(ptr, align)

/**
 * Macro to align a value to a given power-of-two. The resultant
 * value will be of the same type as the first parameter, and
 * will be no lower than the first parameter. Second parameter
 * must be a power-of-two value.
 * This function is the same as RTE_ALIGN_CEIL
 */
#define RTE_ALIGN(val, align) RTE_ALIGN_CEIL(val, align)

/**
 * Macro to align a value to the multiple of given value. The resultant
 * value will be of the same type as the first parameter and will be no lower
 * than the first parameter.
 */
#define RTE_ALIGN_MUL_CEIL(v, mul) \
	(((v + (typeof(v))(mul) - 1) / ((typeof(v))(mul))) * (typeof(v))(mul))

/**
 * Macro to align a value to the multiple of given value. The resultant
 * value will be of the same type as the first parameter and will be no higher
 * than the first parameter.
 */
#define RTE_ALIGN_MUL_FLOOR(v, mul) \
	((v / ((typeof(v))(mul))) * (typeof(v))(mul))

/**
 * Macro to align value to the nearest multiple of the given value.
 * The resultant value might be greater than or less than the first parameter
 * whichever difference is the lowest.
 */
#define RTE_ALIGN_MUL_NEAR(v, mul)				\
	({							\
		typeof(v) ceil = RTE_ALIGN_MUL_CEIL(v, mul);	\
		typeof(v) floor = RTE_ALIGN_MUL_FLOOR(v, mul);	\
		(ceil - v) > (v - floor) ? floor : ceil;	\
	})

/**
 * Checks if a pointer is aligned to a given power-of-two value
 *
 * @param ptr
 *   The pointer whose alignment is to be checked
 * @param align
 *   The power-of-two value to which the ptr should be aligned
 *
 * @return
 *   True(1) where the pointer is correctly aligned, false(0) otherwise
 */
static inline int
rte_is_aligned(void *ptr, unsigned align)
{
	return RTE_PTR_ALIGN(ptr, align) == ptr;
}

/*********** Macros for compile type checks ********/

/**
 * Triggers an error at compilation time if the condition is true.
 */
#define RTE_BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


/**
 * Return pointer to the wrapping struct instance.
 *
 * Example:
 *
 *  struct wrapper {
 *      ...
 *      struct child c;
 *      ...
 *  };
 *
 *  struct child *x = obtain(...);
 *  struct wrapper *w = container_of(x, struct wrapper, c);
 */
#ifndef container_of
#define container_of(ptr, type, member)	__extension__ ({		\
			const typeof(((type *)0)->member) *_ptr = (ptr); \
			__rte_unused type *_target_ptr =	\
				(type *)(ptr);				\
			(type *)(((uintptr_t)_ptr) - offsetof(type, member)); \
		})
#endif


/**
 * Macro to return the minimum of two numbers
 */
#define RTE_MIN(a, b) \
	__extension__ ({ \
		typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a < _b ? _a : _b; \
	})

/**
 * Macro to return the maximum of two numbers
 */
#define RTE_MAX(a, b) \
	__extension__ ({ \
		typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a > _b ? _a : _b; \
	})


/**
 * Get the size of a field in a structure.
 *
 * @param type
 *   The type of the structure.
 * @param field
 *   The field in the structure.
 * @return
 *   The size of the field in the structure, in bytes.
 */
#define RTE_SIZEOF_FIELD(type, field) (sizeof(((type *)0)->field))

#define _RTE_STR(x) #x
/** Take a macro value and get a string version of it */
#define RTE_STR(x) _RTE_STR(x)

/**
 * ISO C helpers to modify format strings using variadic macros.
 * This is a replacement for the ", ## __VA_ARGS__" GNU extension.
 * An empty %s argument is appended to avoid a dangling comma.
 */
#define RTE_FMT(fmt, ...) fmt "%.0s", __VA_ARGS__ ""
#define RTE_FMT_HEAD(fmt, ...) fmt
#define RTE_FMT_TAIL(fmt, ...) __VA_ARGS__

/** Mask value of type "tp" for the first "ln" bit set. */
#define	RTE_LEN2MASK(ln, tp)	\
	((tp)((uint64_t)-1 >> (sizeof(uint64_t) * CHAR_BIT - (ln))))

/** Number of elements in the array. */
#define	RTE_DIM(a)	(sizeof (a) / sizeof ((a)[0]))

/**
 * Converts a numeric string to the equivalent uint64_t value.
 * As well as straight number conversion, also recognises the suffixes
 * k, m and g for kilobytes, megabytes and gigabytes respectively.
 *
 * If a negative number is passed in  i.e. a string with the first non-black
 * character being "-", zero is returned. Zero is also returned in the case of
 * an error with the strtoull call in the function.
 *
 * @param str
 *     String containing number to convert.
 * @return
 *     Number.
 */
static inline uint64_t
rte_str_to_size(const char *str)
{
	char *endptr;
	unsigned long long size;

	while (isspace((int)*str))
		str++;
	if (*str == '-')
		return 0;

	errno = 0;
	size = strtoull(str, &endptr, 0);
	if (errno)
		return 0;

	if (*endptr == ' ')
		endptr++; /* allow 1 space gap */

	switch (*endptr){
	case 'G': case 'g': size *= 1024; /* fall-through */
	case 'M': case 'm': size *= 1024; /* fall-through */
	case 'K': case 'k': size *= 1024; /* fall-through */
	default:
		break;
	}
	return size;
}


#endif //_RTE_COMMON_H_
