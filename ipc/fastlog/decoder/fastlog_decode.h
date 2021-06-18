#ifndef __fastlog_DECODE_h
#define __fastlog_DECODE_h 1

#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <fastlog.h>
#include <sys/time.h>

#include <fastlog_rb.h>
#include <fastlog_list.h>
#include <fastlog_utils.h>
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
    
    rb_node(struct logdata_decode) rb_link_node_rdtsc;   //按时间顺序排序的红黑树
    struct list list_level[FASTLOGLEVELS_NUM];
};

//(默认为ALL)
typedef enum {
    LOG__RANGE_ALL      = 0x0001,       //输出全部
    LOG__RANGE_LEVEL    = 0x0002,       //按级别输出
    LOG__RANGE_NAME     = 0x0004,       //按名称(模块名)输出
    LOG__RANGE_THREAD   = 0x0008,       //按线程输出
    LOG__RANGE_REAPPEAR = 0x0010,       //重现日志 Reappear yesterday
}LOG__RANGE_TYPE;

//(默认为 console|txt)
typedef enum {
    LOG_OUTPUT_FILE_TXT     = 0x0001,   //txt 格式
    LOG_OUTPUT_FILE_JSON    = 0x0002,   //json 格式
    LOG_OUTPUT_FILE_XML     = 0x0004,   //XML 格式
    LOG_OUTPUT_FILE_CONSOLE = 0x0008,   //终端输出
}LOG_OUTPUT_FILE_TYPE;


struct output_struct;

/**
 *  输出操作符
 *
 *  open 初始化
 *  
 */
struct output_operations {
    int (*open)(struct output_struct *o);
    int (*header)(struct output_struct *o, struct fastlog_file_header *header);
    int (*log_item)(struct output_struct *o, struct logdata_decode *logdata, char *log);
    int (*footer)(struct output_struct *o);
    int (*close)(struct output_struct *o);
};

/**
 *  输出实体
 *
 *  range   输出范围
 *  file    输出文件类型
 *  filename    当 file 未置位 LOG_OUTPUT_FILE_CONSOLE 终端时，会检查该文件名
 *              如果为空，则执行失败
 *  ops     对应的操作
 */
struct output_struct {
    bool enable;
    LOG__RANGE_TYPE range;
    union {
        enum FASTLOG_LEVEL level;
        char *name;
        char *thread;
    }range_value;
    
    LOG_OUTPUT_FILE_TYPE file;
    char *filename;
    union {
        FILE *fp;   //对应 CONSOLE 或者 txt
#ifdef FASTLOG_HAVE_LIBXML2
        struct {
            xmlDocPtr doc;
            xmlNodePtr root_node;
            xmlNodePtr header;
            xmlNodePtr body;
            xmlNodePtr footer;
        }xml;
#endif
        //xml 和 json 句柄
    }file_handler;
    struct output_operations *ops;
};

extern struct output_operations output_operations_txt;
extern struct output_operations output_operations_xml;
extern struct output_struct output_txt;
extern struct output_struct output_xml;
/**
 *  fastlog decoder 进程 配置参数， 有默认值，同时支持 getopt 命令行配置
 *  
 */
struct fastlog_decoder_config {
    char *decoder_version;
    
    /* 是否输出详细的日志信息 */
    bool log_verbose_flag;

    /* 启动过程是否打印日志 */
    bool boot_silence;

    /*  */
    char *metadata_file;

    
#define MAX_NUM_LOGDATA_FILES   10
    int nr_log_files;
    char *logdata_file;
#define LOGDATA_FILE_SEPERATOR  ','
    char *other_log_data_files[MAX_NUM_LOGDATA_FILES-1];
    
    bool has_cli;

    int default_log_level;
    
    int output_type;
#define DEFAULT_OUTPUT_FILE "fastlog.txt"
    bool output_filename_isset;
    char* output_filename;
};

// fastlog decoder 配置参数，在 getopt 之后只读
extern struct fastlog_decoder_config decoder_config;


int output_open(struct output_struct *output, char *filename);
int output_header(struct output_struct *output, struct fastlog_file_header *header);
int output_log_item(struct output_struct *output, struct logdata_decode *logdata, char *log); //在`reprintf`中被调用
int output_footer(struct output_struct *output);
int output_close(struct output_struct *output);


void logdata_print_output(struct logdata_decode *logdata, void *arg);
int reprintf(struct logdata_decode *logdata, struct output_struct *output);

int parse_logdata(fastlog_logdata_t *logdata, size_t logdata_size);

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
struct logdata_decode * logdata_rbtree__search(int log_id, uint64_t log_rdtsc);
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
