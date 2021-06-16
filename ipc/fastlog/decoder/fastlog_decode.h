#ifndef __fastlog_DECODE_h
#define __fastlog_DECODE_h 1

#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <fastlog.h>
#include <sys/time.h>

#include <fastlog_rb.h>
#include <fastlog_list.h>
#include <fastlog_cycles.h>


/* 解析出的元数据 */
struct metadata_decode {
    /**
     * `log_id`红黑树节点的索引，见下面函数说明:
     *
     *  metadata_rbtree__cmp:       对比红黑树节点回调函数，内部对比 log_id
     *  metadata_rbtree__search:    查找红黑树节点，传入 log_id(强转，所以log_id必须在此结构起始处)
     */
    unsigned int log_id;
    
    struct fastlog_metadata *metadata;  /* 指向内存映射中的数据 */
    
    struct args_type argsType;      /* 由 format 字符串解析 */
    
    rb_node(struct metadata_decode) rb_link_node;   /* 红黑树节点 */

    /**
     *  以下字符串拆分了 `metadata->string_buf`
     *
     *  如`metadata->string_buf`在内存中 为 TEST\0test.c\0main\0Hello, %s\0task1\0
     *  解析为：
     *  
     *  user_string     = "TEST"
     *  src_filename    = "test.c"
     *  src_function    = "main"
     *  print_format    = "Hello, %s"
     *  thread_name     = "task1"
     *
     *  而字符串长度由`metadata->xxxx_len`决定
     */
    char* user_string; 
    char* src_filename; 
    char* src_function; 
    char* print_format;
    char* thread_name;
};


/* 解析出的日志数据 */
struct logdata_decode {
    
    fastlog_logdata_t *logdata;         //malloc(), 而不是文件映射中的内存, 因为可能将映射的文件 munmap
    
    struct metadata_decode *metadata;   //所对应的源数据
    
    rb_node(struct logdata_decode) rb_link_node_time;   //按时间顺序排序的红黑树
    struct list list_level[FASTLOGLEVELS_NUM];
};


typedef enum {
    LOG_OUTPUT_ALL      = 0x0001,       //输出全部
    LOG_OUTPUT_LEVEL    = 0x0002,       //按级别输出
    LOG_OUTPUT_NAME     = 0x0003,       //按名称输出
    LOG_OUTPUT_REAPPEAR = 0x0004,       //重现日志 Reappear yesterday
}LOG_RANGE_TYPE;

typedef enum {
    LOG_OUTPUT_FILE_TXT,
    LOG_OUTPUT_FILE_JSON,
    LOG_OUTPUT_FILE_XML,
    LOG_OUTPUT_FILE_CONSOLE,
}LOG_OUTPUT_FILE_TYPE;

struct print_operations {
    
};

struct print_struct {
    LOG_RANGE_TYPE          range;
    LOG_OUTPUT_FILE_TYPE    file_type;
    
    struct print_operations *print_ops;
};

int reprintf(struct logdata_decode *logdata);


int release_file(struct fastlog_file_mmap *mapfile);

struct fastlog_file_mmap *meta_mmapfile();
struct fastlog_file_mmap *log_mmapfile();


struct fastlog_file_header *meta_hdr();
struct fastlog_file_header *log_hdr();


void timestamp_tsc_to_string(uint64_t tsc, char str_buffer[32]);


int load_metadata_file(const char *mmapfile_name);
int load_logdata_file(const char *mmapfile_name);

int match_metadata_and_logdata();

int release_metadata_file();
int release_logdata_file();



int64_t level_count(enum FASTLOG_LEVEL level);
int64_t level_count_all();
int64_t meta_count();
int64_t log_count();



/* 元数据红黑树操作函数 */
void metadata_rbtree__init();
void metadata_rbtree__destroy(void (*cb)(struct metadata_decode *node, void *arg), void *arg);
void metadata_rbtree__insert(struct metadata_decode *new_node);
void metadata_rbtree__remove(struct metadata_decode *new_node);
struct metadata_decode * metadata_rbtree__search(int meta_log_id);
void metadata_rbtree__iter(void (*cb)(struct metadata_decode *meta, void *arg), void *arg);


/* 日志数据红黑树操作函数 */
void logdata_rbtree__init();
void logdata_rbtree__destroy(void (*cb)(struct logdata_decode *node, void *arg), void *arg);
void logdata_rbtree__insert(struct logdata_decode *new_node);
void logdata_rbtree__remove(struct logdata_decode *new_node);
struct logdata_decode * logdata_rbtree__search(int meta_log_id);
void logdata_rbtree__iter(void (*cb)(struct logdata_decode *meta, void *arg), void *arg);



/**
 *  级别链表操作 
 *
 *  链表操作不涉及内存分配与释放，`logdata_decode`的`malloc`和`free`在红黑树操作中进行
 */
void level_lists__init();
void level_list__insert(enum FASTLOG_LEVEL level, struct logdata_decode *logdata);
void level_list__remove(enum FASTLOG_LEVEL level, struct logdata_decode *logdata);
void level_list__iter(enum FASTLOG_LEVEL level, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);


#endif /*<__fastlog_DECODE_h>*/
