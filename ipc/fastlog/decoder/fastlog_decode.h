#ifndef __fastlog_DECODE_h
#define __fastlog_DECODE_h 1

#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <regex.h>
#include <stdio.h>
#include <fastlog.h>
#include <sys/time.h>

#include <fastlog_rb.h>
#include <fastlog_list.h>
#include <fastlog_utils.h>
#include <fastlog_cycles.h>

#include <ui/progress.h>

#ifdef FASTLOG_HAVE_LIBXML2

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

#endif /*<FASTLOG_HAVE_LIBXML2>*/

#ifdef FASTLOG_HAVE_JSON

#include <json/json.h>

#endif


//(默认为ALL)
typedef enum {
    LOG__RANGE_TIME     = 0x0001,       //时间(戳)(filter 时 不支持)
    LOG__RANGE_LEVEL    = 0x0002,       //级别    (filter 时 不支持)
    LOG__RANGE_NAME     = 0x0004,       //模块名
    LOG__RANGE_FILE     = 0x0008,       //源文件名    (filter 时 不支持)
    LOG__RANGE_FUNC     = 0x0010,       //函数名
    LOG__RANGE_LINE     = 0x0020,       //行号 (filter 时 不支持)
    LOG__RANGE_THREAD   = 0x0040,       //线程
    LOG__RANGE_CONTENT  = 0x0080,       //内容(单条信息)
    LOG__RANGE_ALL      = 0x00ff,       //全部
    LOG__RANGE_MASK     = LOG__RANGE_ALL,

    /**
     *  Filter 过滤时，支持的 过滤 掩码, 数量需要与`__LOG__RANGE_FILTER_NUM`同步
     *
     *  LOG__RANGE_NAME:    支持模块名过滤
     *  LOG__RANGE_FUNC:    支持函数名过滤
     *  LOG__RANGE_THREAD:  支持线程名过滤
     *  LOG__RANGE_CONTENT: 支持内容过滤
     *
     */
    LOG__RANGE_FILTER_MASK  = LOG__RANGE_NAME|LOG__RANGE_FUNC|LOG__RANGE_THREAD|LOG__RANGE_CONTENT
}LOG__RANGE_TYPE;
    
typedef enum {
    LOG__RANGE_NAME_ENUM,
    LOG__RANGE_FUNC_ENUM,
    LOG__RANGE_THREAD_ENUM,
    LOG__RANGE_CONTENT_ENUM,
    __LOG__RANGE_FILTER_NUM,    /* 注意不要填错，见`LOG__RANGE_FILTER_MASK`注释描述 */
}LOG__RANGE_FILTER_ENUM;


//(默认为 console|txt)
typedef enum {
    /**
     *  输出文件类型
     */
    LOG_OUTPUT_FILE_TXT     = 0x0001,   //txt 格式
    LOG_OUTPUT_FILE_JSON    = 0x0002,   //json 格式
    LOG_OUTPUT_FILE_XML     = 0x0004,   //XML 格式
    LOG_OUTPUT_FILE_CONSOLE = 0x0008,   //终端输出
    LOG_OUTPUT_FILE_MASK    = 0x000f,   //文件输出掩码

    /* `0x0010` - `0x0080` 预留，用于扩展其他文件格式 */

    /**
     *  输出项，见 `struct output_operations`中`header``meta_item``log_item``footer`
     */
    LOG_OUTPUT_ITEM_HDR     = 0x0100,   //头信息项
    LOG_OUTPUT_ITEM_META    = 0x0200,   //元数据信息项
    LOG_OUTPUT_ITEM_LOG     = 0x0400,   //LOG数据信息项
    LOG_OUTPUT_ITEM_FOOT    = 0x0800,   //尾部统计信息项
    LOG_OUTPUT_ITEM_MASK    = 0x0f00,   //信息项掩码
    LOG_OUTPUT_ITEM_LOG_MASK = LOG_OUTPUT_ITEM_HDR|LOG_OUTPUT_ITEM_LOG|LOG_OUTPUT_ITEM_FOOT,
    LOG_OUTPUT_ITEM_META_MASK = LOG_OUTPUT_ITEM_HDR|LOG_OUTPUT_ITEM_META|LOG_OUTPUT_ITEM_FOOT,
}LOG_OUTPUT_TYPE;



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
     *  log_id 链表头
     *
     *  id_list: 所有 log_id 相同的链表头
     *           链表节点为`struct logdata_decode`中的`list_id`
     *
     *  id_cnt:  log_id 的数量统计
     */
    struct list id_list;
    unsigned long id_cnt;

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

/**
 *  用于查找 的红黑树节点
 *  
 */
struct log_search {

    char *string;

    /**
     *  在红黑树的 compare 函数中使用
     *  主要用于 指定是否 使用 子字符串查找(strstr())
     *  在红黑树中的节点，此数值均为 false(并且不会被使用)
     *  在搜索阶段(`log_search_rbtree__search()`)的局部变量
     *    将搜索的该数值设置为 true 进行子字符串查钊
     */
    bool use_strstr;

    LOG__RANGE_FILTER_ENUM string_type;

    
    unsigned long log_cnt;

    /**
     *  保存属于或匹配字符串`string`的日志聊表头节点
     *  对应链表节点在`struct logdata_decode`的`list_search`字段
     */
    struct list log_list_head;
    
    rb_node(struct log_search) rb_link;
};


/* 解析出的日志数据 */
struct logdata_decode {
    
    fastlog_logdata_t *logdata;         //malloc(), 而不是文件映射中的内存, 因为可能将映射的文件 munmap
    
    struct metadata_decode *metadata;   //所对应的源数据
    
    rb_node(struct logdata_decode) rb_link_node_rdtsc;   //按时间顺序排序的红黑树

    /**
     *  日志级别链表
     */
    struct list list_level;

    /**
     *  log_id 链表节点
     *
     *  链表头为 `struct metadata_decode`结构中的`id_list`
     */
    struct list list_id;

    /**
     *  搜索 日志链表
     *  具体请查看`LOG__RANGE_FILTER_ENUM`枚举类型
     *  对应的链表头在`struct log_search`结构体的`log_list_head`字段
     */
    struct list list_search[__LOG__RANGE_FILTER_NUM];
};

struct output_struct;
struct output_filter;

/**
 *  输出操作符
 *
 *  open 初始化
 *  
 */
struct output_operations {

    int (*open)(struct output_struct *o);
    int (*header)(struct output_struct *o, struct fastlog_file_header *header);
    int (*meta_item)(struct output_struct *o, struct metadata_decode *meta);
    int (*log_item)(struct output_struct *o, struct logdata_decode *logdata, char *log);
    int (*footer)(struct output_struct *o);
    int (*close)(struct output_struct *o);
};

/**
 *  过滤器-类型
 *
 *  
 */
typedef enum {
    LOG__FILTER_MATCH_TOTAL = 1,    //完全匹配
    LOG__FILTER_MATCH_STRSTR,       //子串匹配，使用 `strstr()`
    LOG__FILTER_REGEX,              //正则表达式(暂不支持)
}LOG__FILTER_TYPE;


/**
 *  用于传递参数
 */
struct output_filter_arg {
    union {
        char *func;
        char *name;
        char *thread;
        char *content;
        char *value;
    };
    char *log_buffer;
};

static const struct output_filter_arg output_filter_arg_null = {{NULL}, NULL};

/**
 *  过滤器
 */
struct output_filter {
    
    LOG__RANGE_TYPE log_range;
    
    LOG__FILTER_TYPE filter_type;

    union {
        bool (*match_prefix_ok)(struct output_filter *, struct logdata_decode *, char *value);
        bool (*match_log_content_ok)(struct output_filter *, struct logdata_decode *, char *log, char *value);
    };
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
    const bool enable;
    
    LOG_OUTPUT_TYPE file;
    char *filename;
    union {
        FILE *fp;   //对应 CONSOLE 或者 txt
#ifdef FASTLOG_HAVE_LIBXML2
        struct {
            xmlDocPtr doc;
            xmlNodePtr root_node;
            xmlNodePtr header;
            xmlNodePtr header_metadata;
            xmlNodePtr body;
            xmlNodePtr footer;
        }xml;
#endif
#ifdef FASTLOG_HAVE_JSON
        struct {
            json_object *root;  //如果使用文件输出，才会使用该字段
            json_object *header;
            json_object *metadata;
            json_object *logdata;
            json_object *footer;
        }json;
#endif
    //xml 和 json 句柄
    }file_handler;

    /* 输出时进行统计 */
    unsigned long output_meta_cnt;
    unsigned long output_log_cnt;

    /**
     *  输出操作
     *
     *  支持 txt, xml, json 等格式的输出
     */
    struct output_operations *ops;

    /**
     *  过滤器
     */
    int filter_num;
    struct output_filter_arg filter_arg[__LOG__RANGE_FILTER_NUM];
    struct output_filter *filter[__LOG__RANGE_FILTER_NUM];
};

extern struct output_struct output_txt;
extern struct output_struct output_xml;
extern struct output_struct output_json;

extern struct output_filter filter_name;
extern struct output_filter filter_func;
extern struct output_filter filter_thread;
extern struct output_filter filter_content;

extern progress_t pro_bar;

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

    /* Filter 信息 */
    char *match_name;
    char *match_func;
    char *match_thread;
    char *match_content;

    /* 统计信息 */
    unsigned long total_fmeta_num;  /* 从元数据文件中获取的元数据个数 */
    unsigned long total_flog_num;   /* 从日志数据文件中获取的日志数据个数 */
    
};

// fastlog decoder 配置参数，在 getopt 之后只读
extern struct fastlog_decoder_config decoder_config;


int output_open(struct output_struct *output, char *filename);
int output_header(struct output_struct *output, struct fastlog_file_header *header);
int output_log_item(struct output_struct *output, struct logdata_decode *logdata, char *log); //在`reprintf`中被调用
int output_footer(struct output_struct *output);
int output_close(struct output_struct *output);

int output_setfilter(struct output_struct *output, struct output_filter *filter, struct output_filter_arg arg);
bool output_callfilter(struct output_struct *output, struct logdata_decode *logdata);
int output_updatefilter_arg(struct output_struct *output, char *log_buffer);
int output_clearfilter(struct output_struct *output);

void progress_output(struct output_struct *output, unsigned long i, unsigned long total_num);

void output_metadata(struct metadata_decode *meta, void *arg);
void output_logdata(struct logdata_decode *logdata, void *arg);
int reprintf(struct logdata_decode *logdata, struct output_struct *output);

int parse_logdata(fastlog_logdata_t *logdata, size_t logdata_size);

int release_file(struct fastlog_file_mmap *mapfile);

struct fastlog_file_mmap *meta_mmapfile();
struct fastlog_file_mmap *log_mmapfile();


struct fastlog_file_header *meta_hdr();
struct fastlog_file_header *log_hdr();


void timestamp_tsc_to_string(uint64_t tsc, char str_buffer[32]);


int load_metadata_file(char *mmapfile_name);
int load_logdata_file(char *mmapfile_name);

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
void metadata_rbtree__iter_level(enum FASTLOG_LEVEL level, void (*cb)(struct metadata_decode *, void *arg), void *arg);


/* 日志数据红黑树操作函数 */
void logdata_rbtree__init();
void logdata_rbtree__destroy(void (*cb)(struct logdata_decode *node, void *arg), void *arg);
void logdata_rbtree__insert(struct logdata_decode *new_node);
void logdata_rbtree__remove(struct logdata_decode *new_node);
struct logdata_decode * logdata_rbtree__search(int log_id, uint64_t log_rdtsc);
void logdata_rbtree__iter(void (*cb)(struct logdata_decode *meta, void *arg), void *arg);

/* 日志搜索红黑树操作 */
void log_search_rbtree__init();
void log_search_rbtree__destroy(LOG__RANGE_FILTER_ENUM type, void (*cb)(struct log_search *node, void *arg), void *arg);
void log_search_rbtree__destroyall(void (*cb)(struct log_search *node, void *arg), void *arg);
void log_search_rbtree__insert(LOG__RANGE_FILTER_ENUM type, struct log_search *new_node);
void log_search_rbtree__remove(LOG__RANGE_FILTER_ENUM type, struct log_search *new_node);
struct log_search *log_search_rbtree__search(LOG__RANGE_FILTER_ENUM type, char *string);
struct log_search * log_search_rbtree__search2(LOG__RANGE_FILTER_ENUM type, char *string);  
struct log_search *log_search_rbtree__search_or_create(LOG__RANGE_FILTER_ENUM type, char *string);
void log_search_rbtree__iter(LOG__RANGE_FILTER_ENUM type, void (*cb)(struct log_search *meta, void *arg), void *arg);



/**
 *  级别链表操作 
 *
 *  链表操作不涉及内存分配与释放，`logdata_decode`的`malloc`和`free`在红黑树操作中进行
 */
void level_lists__init();
void level_list__insert(enum FASTLOG_LEVEL level, struct logdata_decode *logdata);
void level_list__remove(enum FASTLOG_LEVEL level, struct logdata_decode *logdata);
void level_list__iter(enum FASTLOG_LEVEL level, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);

/**
 *  log_id 的bitmask 
 *
 *  用户统计 log_id 信息
 */
void log_ids__init();
void log_ids__destroy();
void log_ids__set(int log_id);
int  log_ids__isset(int log_id);
int  log_ids__first();
int  log_ids__next(int log_id);
int  log_ids__last();
void log_ids__iter(void (*cb)(int log_id, void* arg), void *arg);


/**
 *  每个元数据中，都存有一个 log_id 的链表
 *
 *  XXX_raw 使用原始`struct metadata_decode`结构操作
 *  XXX     使用`log_id`操作
 */
void id_lists__init_raw(struct metadata_decode *metadata);
void id_list__insert_raw(struct metadata_decode *metadata, struct logdata_decode *logdata);
void id_list__remove_raw(struct metadata_decode *metadata, struct logdata_decode *logdata);
int id_list__iter_raw(struct metadata_decode *metadata, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);

void id_lists__init(int log_id);
void id_list__insert(int log_id, struct logdata_decode *logdata);
void id_list__remove(int log_id, struct logdata_decode *logdata);
int id_list__iter(int log_id, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);


void log_search_list__insert(struct log_search *node, struct logdata_decode *logdata);
void log_search_list__remove(struct log_search *node, struct logdata_decode *logdata);
void log_search_list__iter(struct log_search *node, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);
void log_search_list__iter2(LOG__RANGE_FILTER_ENUM type, char *string, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);
void log_search_list__iter3(LOG__RANGE_FILTER_ENUM type, char *string, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg);


#endif /*<__fastlog_DECODE_h>*/
