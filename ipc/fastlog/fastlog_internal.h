/**
 *  FastLog 低时延/高吞吐 LOG日志系统
 *
 *
 *  Author: Rong Tao
 *  data: 2021年6月2日 - 
 */

#ifndef __fAStLOG_INTERNAL_H
#define __fAStLOG_INTERNAL_H 1

#ifndef __fAStLOG_H
#error "You can't include <fastlog_internal.h> directly, #include <fastlog.h> instead."
#endif

#if !defined(__x86_64__) && defined(__i386__)
#error "Just support x86"
#endif

#define __USE_GNU
#define _GNU_SOURCE
#include <sched.h>
#include <ctype.h>
#include <string.h>
#include <syscall.h>
#include <stdbool.h>
#include <sched.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <printf.h> //parse_printf_format() 和 


#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
#define __cachelinealigned __attribute__((aligned(64)))
#define _unused             __attribute__((unused))


#ifndef _NR_CPU
#define _NR_CPU    32
#endif

#ifndef _FASTLOG_MAX_NR_ARGS
#define _FASTLOG_MAX_NR_ARGS    15
#endif


#if _FASTLOG_MAX_NR_ARGS < 15
#error "_FASTLOG_MAX_NR_ARGS must bigger than 15, -D_FASTLOG_MAX_NR_ARGS=15"
#endif



/* 参数类型 */
enum format_arg_type {
    FAT_NONE,
    FAT_INT8,
    FAT_INT16,
    FAT_INT32,
    FAT_INT64,
    FAT_UINT8,
    FAT_UINT16,
    FAT_UINT32,
    FAT_UINT64,
    FAT_INT,
    FAT_SHORT,
    FAT_SHORT_INT,
    FAT_LONG,
    FAT_LONG_INT,
    FAT_LONG_LONG,
    FAT_LONG_LONG_INT,
    FAT_CHAR,
    FAT_UCHAR,
    FAT_STRING,
    FAT_POINTER,
    FAT_FLOAT,
    FAT_DOUBLE,
    FAT_LONG_DOUBLE,
    FAT_MAX_FORMAT_TYPE
};



/* 参数类型对应的长度 */
enum format_arg_type_size {
    SIZEOF_FAT_INT8   =   sizeof(int8_t),
    SIZEOF_FAT_INT16  =   sizeof(int16_t),
    SIZEOF_FAT_INT32  =   sizeof(int32_t),
    SIZEOF_FAT_INT64  =   sizeof(int64_t),
    SIZEOF_FAT_UINT8  =   sizeof(uint8_t),
    SIZEOF_FAT_UINT16 =   sizeof(uint16_t),
    SIZEOF_FAT_UINT32 =   sizeof(uint32_t),
    SIZEOF_FAT_UINT64 =   sizeof(uint64_t),
    SIZEOF_FAT_INT    =   sizeof(int),
    SIZEOF_FAT_SHORT  =   sizeof(short),
    SIZEOF_FAT_SHORT_INT  =   sizeof(short int),
    SIZEOF_FAT_LONG       =   sizeof(long),
    SIZEOF_FAT_LONG_INT   =   sizeof(long int),
    SIZEOF_FAT_LONG_LONG  =   sizeof(long long),
    SIZEOF_FAT_LONG_LONG_INT  =   sizeof(long long int),
    SIZEOF_FAT_CHAR   =   sizeof(char),
    SIZEOF_FAT_UCHAR  =   sizeof(unsigned char),
    SIZEOF_FAT_STRING =   -1,
    SIZEOF_FAT_POINTER    =   sizeof(void*),
    SIZEOF_FAT_FLOAT  =   sizeof(float),
    SIZEOF_FAT_DOUBLE =   sizeof(double),
    SIZEOF_FAT_LONG_DOUBLE    =   sizeof(long double),
};

static const struct {
    size_t size;
    char *name;
} FASTLOG_FAT_TYPE2SIZENAME[FAT_MAX_FORMAT_TYPE] = {
#define _TYPE_SIZE(T)   [T] = {SIZEOF_##T, #T}
    _TYPE_SIZE(FAT_INT8),
    _TYPE_SIZE(FAT_INT16),
    _TYPE_SIZE(FAT_INT32),
    _TYPE_SIZE(FAT_INT64),
    _TYPE_SIZE(FAT_UINT8),
    _TYPE_SIZE(FAT_UINT16),
    _TYPE_SIZE(FAT_UINT32),
    _TYPE_SIZE(FAT_UINT64),
    _TYPE_SIZE(FAT_INT),
    _TYPE_SIZE(FAT_SHORT),
    _TYPE_SIZE(FAT_SHORT_INT),
    _TYPE_SIZE(FAT_LONG),
    _TYPE_SIZE(FAT_LONG_INT),
    _TYPE_SIZE(FAT_LONG_LONG),
    _TYPE_SIZE(FAT_LONG_LONG_INT),
    _TYPE_SIZE(FAT_CHAR),
    _TYPE_SIZE(FAT_UCHAR),
    _TYPE_SIZE(FAT_STRING),
    _TYPE_SIZE(FAT_POINTER),
    _TYPE_SIZE(FAT_FLOAT),
    _TYPE_SIZE(FAT_DOUBLE),
    _TYPE_SIZE(FAT_LONG_DOUBLE),
#undef _TYPE_SIZE    
};


struct fastlog_log_metadata {
    int log_id;
    int log_level:4;
    int log_line:16;        // __LINE__ 宏最大 65535 行
    int metadata_size:12;   //此数据结构 所占大小
    char *src_file;
    char *src_private_name;
    char *print_format;
}__attribute__((packed));

/**
 *  例
 *  printf("%d %s %f %llf", ...); 
 *  >>将被解析为>> 
 *  nargs = 4
 *  argtype[0] = FAT_INT
 *  argtype[1] = FAT_STRING
 *  argtype[2] = FAT_DOUBLE
 *  argtype[3] = FAT_LONG_DOUBLE
 */
struct args_type {
    int nargs:8;    //argtype 的使用个数
    uint8_t argtype[_FASTLOG_MAX_NR_ARGS];  // enum format_arg_type 之一
};

struct arg_hdr {
    int log_id; //所属ID
    int log_args_size;
    uint64_t log_rdtsc;
    char log_args_buff[];   //存入 参数
};


#define __FAST_LOG(arg_size, level, name, format, ...) do {                                         \
    /* initial LOG ID */                                                                            \
    static int __thread log_id = 0;                                                                 \
    static char __thread args_buffer[arg_size];                                                     \
    static struct args_type __thread args = {0,0,{0}};                                              \
    if(unlikely(!log_id)) {                                                                         \
        if(!__builtin_constant_p(level)) assert(0 && "level must be one of enum FASTLOG_LEVEL");    \
        if(!__builtin_constant_p(name)) assert(0 && "name must be const variable");                 \
        if(!__builtin_constant_p(format)) assert(0 && "Just support static string.");               \
        log_id = __fastlog_get_unused_logid(level, name, __FILE__, __func__, __LINE__, format);        \
        __fastlog_parse_format(format, &args);                                                      \
    }                                                                                               \
    if (false) { __fastlog_check_format(format, ##__VA_ARGS__); }                                   \
    __fastlog_print_buffer(log_id, args_buffer, &args, ##__VA_ARGS__);\
    __fastlog_print_parse_buffer(args_buffer, &args);/* 此处应替换为对应的 保存参数的接口 */\
}while(0)
    //TODO
        //    printf("[printf] "format, ##__VA_ARGS__);\



/* 编译阶段检测 printf 格式 */
static void __attribute__((format (printf, 1, 2)))
__fastlog_check_format(const char *fmt, ...)
{
}

/* 获取 当前 CPU */
static inline int 
__fastlog_sched_getcpu() 
{ 
    return sched_getcpu(); 
}

/* 同上 */
static inline unsigned 
__fastlog_getcpu()
{
    unsigned cpu, node;
    int ret = syscall(__NR_getcpu, &cpu, &node);
    return cpu;
}

static inline __attribute__((always_inline)) uint64_t 
__fastlog_rdtsc()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo);
}





int __fastlog_get_unused_logid(int level, const char *name, char *file, char *func, int line, const char *format);
int __fastlog_parse_format(const char *fmt, struct args_type *args);
int __fastlog_print_parse_buffer(char *buffer, struct args_type *args);



/* 通过 */
static inline int __fastlog_print_buffer(int log_id, char *buffer, struct args_type *args, ...)
{
    int size = 0;
    int iarg;
    va_list va;
    va_start(va, args);

    struct arg_hdr *arghdr = buffer;
    buffer = arghdr->log_args_buff;
    
    arghdr->log_id = log_id;

    for(iarg=0; iarg<args->nargs; iarg++) {

//        printf("%d->%d(%s)\n", iarg, args->argtype[iarg], FASTLOG_FAT_TYPE2SIZENAME[args->argtype[iarg]].name);
        
        switch(args->argtype[iarg]) {

#define _CASE(fat_type, type)                       \
        case fat_type: {                            \
            type _val = va_arg(va, type);           \
            memcpy(buffer, &_val, sizeof(type));    \
            buffer += sizeof(type);                 \
            size += sizeof(type);                   \
            break;                                  \
        }
#define _CASE_STRING(fat_type, type)                \
        case fat_type: {                            \
            type _val = va_arg(va, type);           \
            int len = strlen(_val);                 \
            memcpy(buffer, _val, len);              \
            buffer += len;                          \
            size += len;                            \
            break;                                  \
        }
#define _CASE_POINTER(fat_type, type)                   \
        case fat_type: {                                \
            type _val = va_arg(va, type);               \
            unsigned long __v = (unsigned long)_val;    \
            memcpy(buffer, &__v, sizeof(type));         \
            buffer += sizeof(type);                     \
            size += sizeof(type);                       \
            break;                                      \
        }

        _CASE(FAT_INT8, int8_t);
        _CASE(FAT_INT16, int16_t);
        _CASE(FAT_INT32, int32_t);
        _CASE(FAT_INT64, int64_t);
        
        _CASE(FAT_UINT8, uint8_t);
        _CASE(FAT_UINT16, uint16_t);
        _CASE(FAT_UINT32, uint32_t);
        _CASE(FAT_UINT64, uint64_t);
        
        _CASE(FAT_INT, int);
        _CASE(FAT_SHORT, short);
        _CASE(FAT_SHORT_INT, short int);

        _CASE(FAT_LONG, long);
        _CASE(FAT_LONG_INT, long int);
        _CASE(FAT_LONG_LONG, long long);
        _CASE(FAT_LONG_LONG_INT, long long int);

        _CASE(FAT_CHAR, char);
        _CASE(FAT_UCHAR, unsigned char);
        
        _CASE_STRING(FAT_STRING, char *);

        _CASE_POINTER(FAT_POINTER, void*);
        
        _CASE(FAT_FLOAT, double);
        _CASE(FAT_DOUBLE, double);
        _CASE(FAT_LONG_DOUBLE, long double);
        
        default:
            printf("\t%d unknown(%d).\n", iarg, args->argtype[iarg]);
            assert(0 && "Not support type." && __FILE__ && __LINE__);
            break;
#undef _CASE
#undef _CASE_STRING
#undef _CASE_POINTER
        }
    
    }

    va_end(va);

    arghdr->log_args_size = size;
    arghdr->log_rdtsc = __fastlog_rdtsc();

//    printf("size = %d\n", size);
    return size;
}





#endif /*<__fAStLOG_INTERNAL_H>*/
