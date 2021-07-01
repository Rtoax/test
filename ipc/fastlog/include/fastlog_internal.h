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

#ifndef __USE_GNU
#define __USE_GNU 1
#endif
#define _GNU_SOURCE
#include <sched.h>
#include <sys/eventfd.h> //eventfd

#include <ctype.h>
#include <string.h>
#include <syscall.h>
#include <stdbool.h>
#include <sched.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>   //系统调用 time(2)
#include <string.h>
#include <syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <printf.h> //parse_printf_format() 和 
#include <sys/utsname.h>    //uname


#include <fastlog_utils.h>
#include <fastlog_staging_buffer.h>


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
    SIZEOF_FAT_STRING =   0,
    SIZEOF_FAT_POINTER    =   sizeof(void*),
    SIZEOF_FAT_FLOAT  =   sizeof(float),
    SIZEOF_FAT_DOUBLE =   sizeof(double),
    SIZEOF_FAT_LONG_DOUBLE    =   sizeof(long double),
};

#define SIZEOF_FAT(t) SIZEOF_##t

static const struct {
    size_t size;
    char *name;
} FASTLOG_FAT_TYPE2SIZENAME[FAT_MAX_FORMAT_TYPE] = {
#define _TYPE_SIZE(T)   [T] = {SIZEOF_FAT(T), #T}
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


//文件映射
struct fastlog_file_mmap {
    enum {
        FILE_MMAP_NULL,     //未映射
        FILE_MMAP_MMAPED,   //已经映射
    } status;
    char *filepath;
    int fd;
    size_t mmap_size;
    char *mmapaddr;
};


enum {
    __FATSLOG_MAGIC1 = 0x1234abcd,
    __FATSLOG_MAGIC2,
    __FATSLOG_MAGIC3,
    __FATSLOG_MAGIC4,
    __FATSLOG_MAGIC5,
    
};

/**
 *  元数据头信息
 */
struct fastlog_file_header {

#define FATSLOG_METADATA_HEADER_MAGIC_NUMBER    __FATSLOG_MAGIC1
#define FATSLOG_METADATA_FILE_DEFAULT           "fastlog.metadata"
#define FATSLOG_METADATA_FILE_SIZE_DEFAULT      (1024*1024*10) //10MB


#define FATSLOG_LOG_HEADER_MAGIC_NUMBER     __FATSLOG_MAGIC2
#define FATSLOG_LOG_FILE_DEFAULT            "fastlog.log"
#define FATSLOG_LOG_FILE_SIZE_DEFAULT       (1024*1024*10) 

#define FASTLOG_LOG_FILE_ENDIAN_MAGIC   0x12345678

    /**
     *  字节序
     *  当解析该日志文件的服务器与生成日志文件的服务器 endian 不同，不可解析
     *  2021年6月30日09:37:41
     */
    unsigned int endian;

    unsigned int magic;
    uint64_t cycles_per_sec;
    uint64_t start_rdtsc;
    time_t unix_time_sec;       //1970.1.1 00:00:00 至今 秒数(time(2))
    struct utsname unix_uname;  //系统信息(uname(2))

    unsigned long data_num;
    
    /**
     *  时间戳，LOG数，统计信息
     */
    char data[];
}__attribute__((packed));

/**
 *  单条元数据信息
 */
struct fastlog_metadata {
#define FATSLOG_METADATA_MAGIC_NUMBER    __FATSLOG_MAGIC3
    unsigned int magic;
    unsigned int log_id;
    unsigned int log_level:4;
    unsigned int log_line:28;           // __LINE__ 宏最大 65535 行
    unsigned int metadata_size:16;      //元数据 所占大小

    /* 标记下面内存中各个字符串的长度 */
    unsigned int user_string_len:8; 
    unsigned int src_filename_len:8; 
    unsigned int src_function_len:8; 
    unsigned int print_format_len:8;
    unsigned int thread_name_len:8;

    /**
     *  `fastlog_metadata->string_buf`保存多个字符串
     *
     *  如`fastlog_metadata->string_buf`在内存中 为 TEST\0test.c\0main\0Hello, %s\0task1\0
     *  
     *  拆分结果在数据结构`struct metadata_decode`被解析为：
     *  
     *  metadata_decode->user_string     = "TEST"
     *  metadata_decode->src_filename    = "test.c"
     *  metadata_decode->src_function    = "main"
     *  metadata_decode->print_format    = "Hello, %s"
     *  metadata_decode->thread_name     = "task1"
     *
     *  而字符串长度由`fastlog_metadata->xxxx_len`决定
     */
    char string_buf[]; //保存 源文件名 和 格式化字符串
}__attribute__((packed));

/**
 *  单条LOG args类型信息
 *
 *  格式如下：
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
    unsigned int nargs:7;        //argtype 的使用个数
    unsigned int has_string:1;   //只有存在 字符串时，参数总大小才不固定
    unsigned int pre_size:8;     //参数总size：如果 没有字符串`has_string=0`时该字段生效
    uint8_t argtype[_FASTLOG_MAX_NR_ARGS];  // enum format_arg_type 之一
#define ARGS_TYPE_INITIALIZER     {0u,0u,0u,{0}}  
};

/**
 *  单条LOG信息
 *  
 *  该数据结构应尽可能的小，以降低每条日志过程的数据量
 */
typedef struct arg_hdr {
    int log_id:24; //所属ID
    int log_args_size:8;
    uint64_t log_rdtsc;
    char log_args_buff[];   //存入 参数
}__attribute__((packed)) fastlog_logdata_t;



#define __FAST_LOG(level, name, format, ...) do {                                                   \
    if(fastlog_getlevel() < level) break; /* level enable the LOG */                                \
    /* initial LOG ID */                                                                            \
    static int __thread log_id = 0;                                                                 \
    static struct args_type __thread args = ARGS_TYPE_INITIALIZER;                                  \
    if(unlikely(!log_id)) {                                                                         \
        if(!__builtin_constant_p(level)) assert(0 && "level must be one of enum FASTLOG_LEVEL");    \
        if(!__builtin_constant_p(name)) assert(0 && "name must be const variable");                 \
        if(!__builtin_constant_p(format)) assert(0 && "Just support static string.");               \
        log_id = __fastlog_get_unused_logid(level, name, __FILE__, __func__, __LINE__, format);     \
        __fastlog_parse_format(format, &args);                                                      \
                                                                                                    \
        /* 确保缓冲区不为空  */                                                                             \
        ensureStagingBufferAllocated();                                                             \
    }                                                                                               \
    if (unlikely(false)) { __fastlog_check_format(format, ##__VA_ARGS__); }                         \
    __fastlog_print_buffer(log_id, &args, ##__VA_ARGS__);                                           \
}while(0)



/* 编译阶段检测 printf 格式 */
static void __attribute__((format (printf, 1, 2))) _unused
__fastlog_check_format(const char *fmt, ...)
{
}
fl_inline int 
__fastlog_print_buffer(int log_id, struct args_type *args, ...);


int __fastlog_get_unused_logid(int level, const char *name, const char *file, const char *func, int line, const char *format);
int __fastlog_parse_format(const char *fmt, struct args_type *args);


int mmap_fastlog_logfile_write(struct fastlog_file_mmap *mmap_file, char *filename, char *backupfilename, size_t size);
void msync_fastlog_logfile_write(struct fastlog_file_mmap *mmap_file);
int mmap_fastlog_logfile_read(struct fastlog_file_mmap *mmap_file, char *filename);
int unmmap_fastlog_logfile(struct fastlog_file_mmap *mmap_file);

#endif /*<__fAStLOG_INTERNAL_H>*/
