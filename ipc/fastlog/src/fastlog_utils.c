#include <stdio.h>

#include <fastlog.h>

#include <fastlog_utils.h>
#include <fastlog_internal.h>

typedef enum {
     _ANSI_NONE,
     _ANSI_GRAY,
     _ANSI_CYAN,
     _ANSI_PURPLE,
     _ANSI_BLUE,
     _ANSI_YELLOW,
     _ANSI_GREEN,
     _ANSI_RED,
     _ANSI_BOLD ,
     _ANSI_SHINE,
     _ANSI_BLINK,
     _ANSI_RESET,
} color_type;
     
#define _ANSI_NONE_STR       ""
#define _ANSI_GRAY_STR       "\x1b[1;37;40m"
#define _ANSI_CYAN_STR       "\x1b[1;36m"
#define _ANSI_PURPLE_STR     "\x1b[1;35m"
#define _ANSI_BLUE_STR       "\x1b[1;34m"
#define _ANSI_YELLOW_STR     "\x1b[1;33m"
#define _ANSI_GREEN_STR      "\x1b[1;32m"
#define _ANSI_RED_STR        "\x1b[1;31m"
#define _ANSI_BOLD_STR       "\x1b[1m"
#define _ANSI_SHINE_STR      "\x1b[1;35m" /*"\x1b[1;5;35m"*/
#define _ANSI_BLINK_STR      "\x1b[1;4;5m"
     
#define _ANSI_RESET_STR      "\x1b[0m"

#define _ITEM(e) [e] = e##_STR
const static char _unused *COLORS[] = {
    _ITEM(_ANSI_NONE),
    _ITEM(_ANSI_GRAY),
    _ITEM(_ANSI_CYAN),
    _ITEM(_ANSI_PURPLE),
    _ITEM(_ANSI_BLUE),
    _ITEM(_ANSI_YELLOW),
    _ITEM(_ANSI_GREEN),
    _ITEM(_ANSI_RED),
    _ITEM(_ANSI_BOLD),
    _ITEM(_ANSI_SHINE),
    _ITEM(_ANSI_BLINK),
    _ITEM(_ANSI_RESET),
};
#undef _ITEM


static const char _unused* FASTLOG_LEVEL_NAME[] = {
    "unknown", 
    "CRIT", 
    "ERRO",
    "WARN", 
    "INFO", 
    "DEBG",
    "unknown", 
    "unknown"
};
static const char* FASTLOG_LEVEL_NAME_COLORFUL[] = {
    "unknown",
    _ANSI_PURPLE_STR"CRIT"_ANSI_RESET_STR, 
    _ANSI_RED_STR"ERRO"_ANSI_RESET_STR,
    _ANSI_YELLOW_STR"WARN"_ANSI_RESET_STR, 
    _ANSI_BLUE_STR"INFO"_ANSI_RESET_STR, 
    _ANSI_BOLD_STR"DEBG"_ANSI_RESET_STR, 
    "unknown",
    "unknown"
};

const char *strlevel(enum FASTLOG_LEVEL level)
{
    if(level < FASTLOG_CRIT || level >= FASTLOGLEVELS_NUM)
        return "unknown";

    return FASTLOG_LEVEL_NAME[level];
}

const char *strlevel_color(enum FASTLOG_LEVEL level)
{
    if(level < FASTLOG_CRIT || level >= FASTLOGLEVELS_NUM)
        return _ANSI_GRAY_STR"unknown"_ANSI_RESET_STR;

    return FASTLOG_LEVEL_NAME_COLORFUL[level];
}





fl_inline int  
fastlog_atomic64_cmpset(volatile uint64_t *dst, uint64_t exp, uint64_t src)
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

fl_inline void 
fastlog_atomic64_init(fastlog_atomic64_t *v)
{
	fastlog_atomic64_cmpset((volatile uint64_t *)&v->cnt, v->cnt, 0);
}

fl_inline int64_t 
fastlog_atomic64_read(fastlog_atomic64_t *v)
{
    return v->cnt;
}

fl_inline void 
fastlog_atomic64_add(fastlog_atomic64_t *v, int64_t inc) 
{
	asm volatile(
			"lock ; "
			"addq %[inc], %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: [inc] "ir" (inc),     /* input */
			  "m" (v->cnt)
			);
}

fl_inline void 
fastlog_atomic64_inc(fastlog_atomic64_t *v)
{
	asm volatile(
			"lock ; "
			"incq %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: "m" (v->cnt)          /* input */
			);
}

fl_inline void
fastlog_atomic64_dec(fastlog_atomic64_t *v)
{
	asm volatile(
			"lock ; "
			"decq %[cnt]"
			: [cnt] "=m" (v->cnt)   /* output */
			: "m" (v->cnt)          /* input */
			);
}



/* 获取 当前 CPU */
fl_inline int 
__fastlog_sched_getcpu() 
{ 
    return sched_getcpu(); 
}

/* 同上 */
fl_inline unsigned 
__fastlog_getcpu()
{
    unsigned cpu, node;
    int _unused ret = syscall(__NR_getcpu, &cpu, &node);
    return cpu;
}

fl_inline  uint64_t 
__fastlog_rdtsc()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo);
}

