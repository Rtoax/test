#define _GNU_SOURCE
#include <fastlog_decode.h>


static fastlog_atomic64_t metadata_count;   /* 元数据统计 */
static fastlog_atomic64_t logdata_count;   /* 日志数据统计 */

/**
 *  元数据红黑树 函数
 *
 *  rb_proto:  元数据 红黑树 函数声明
 *  rb_gen:  元数据 红黑树 函数定义
 *
 *  metadata_rbtree__cmp: metadata_decode 结构对比
 *
 *  metadata_rbtree__init:  红黑树初始化 
 *  metadata_rbtree__destroy:   红黑树销毁
 *  metadata_rbtree__insert:  红黑树插入
 *  metadata_rbtree__remove:  红黑树删除
 *  metadata_rbtree__search:   红黑树查找
 *  metadata_rbtree__iter:  红黑树遍历 
 */
typedef rb_tree(struct metadata_decode) metadata_rbtree_t;

/**
 *  元数据红黑树
 *
 *  节点 struct metadata_decode->rb_link_node
 */
static metadata_rbtree_t metadata_rbtree;

rb_proto(static, metadata_rbtree_, metadata_rbtree_t, struct metadata_decode);

static int metadata_rbtree__cmp(const struct metadata_decode *a_node, const struct metadata_decode *a_other)
{
    if(a_node->log_id <  a_other->log_id) return -1;
    else if(a_node->log_id == a_other->log_id) return 0;
    else if(a_node->log_id >  a_other->log_id) return 1;
    return 0;
}


int64_t meta_count()
{
    return fastlog_atomic64_read(&metadata_count);
}


void metadata_rbtree__init()
{
    metadata_rbtree_new(&metadata_rbtree);
    fastlog_atomic64_init(&metadata_count);
}
void metadata_rbtree__destroy(void (*cb)(struct metadata_decode *node, void *arg), void *arg)
{
    assert(cb && "NULL callback error.");
    
    metadata_rbtree_destroy(&metadata_rbtree, cb, arg);
}

void metadata_rbtree__insert(struct metadata_decode *new_node)
{
    metadata_rbtree_insert(&metadata_rbtree, new_node);
    fastlog_atomic64_inc(&metadata_count);
}

void metadata_rbtree__remove(struct metadata_decode *new_node)
{
    metadata_rbtree_remove(&metadata_rbtree, new_node);
    fastlog_atomic64_dec(&metadata_count);
}

struct metadata_decode * metadata_rbtree__search(int meta_log_id)
{
    return metadata_rbtree_search(&metadata_rbtree, (const struct metadata_decode*)&meta_log_id);
}

void metadata_rbtree__iter_level(enum FASTLOG_LEVEL level, void (*cb)(struct metadata_decode *meta, void *arg), void *arg)
{
    assert(cb && "NULL callback error.");
    
    struct metadata_decode *_next = NULL;

    for(_next = metadata_rbtree_first(&metadata_rbtree); 
        _next; 
        _next = metadata_rbtree_next(&metadata_rbtree, _next)) {
        //所有级别
        if(FASTLOGLEVEL_ALL == level) {
            cb(_next, arg);
        } else {
            //级别匹配
            if(level == _next->metadata->log_level) {
                cb(_next, arg);
            } else continue;
        }
    }
}


void metadata_rbtree__iter(void (*cb)(struct metadata_decode *meta, void *arg), void *arg)
{
    metadata_rbtree__iter_level(FASTLOGLEVEL_ALL, cb, arg);
}


rb_gen(static _unused, metadata_rbtree_, metadata_rbtree_t, struct metadata_decode, rb_link_node, metadata_rbtree__cmp);



/**
 *  日志数据红黑树 函数
 *
 *  rb_proto: 日志数据 红黑树 函数声明
 *  rb_gen: 日志数据 红黑树 函数定义
 *
 *  logdata_rbtree__cmp: logdata_decode 结构比较
 *
 *  logdata_rbtree__init:  红黑树初始化 
 *  logdata_rbtree__destroy:   红黑树销毁
 *  logdata_rbtree__insert:  红黑树插入
 *  logdata_rbtree__remove:  红黑树删除
 *  logdata_rbtree__search:   红黑树查找
 *  logdata_rbtree__iter:  红黑树遍历 
 */

typedef rb_tree(struct logdata_decode) logdata_rbtree_t;

static logdata_rbtree_t logdata_rbtree;

rb_proto(static, logdata_rbtree_, logdata_rbtree_t, struct logdata_decode);

static int logdata_rbtree__cmp(const struct logdata_decode *a_node, const struct logdata_decode *a_other)
{
    if(a_node->logdata->log_rdtsc <  a_other->logdata->log_rdtsc) {
        return -1;
    } else if(a_node->logdata->log_rdtsc == a_other->logdata->log_rdtsc) {
        if(a_node->logdata->log_id < a_other->logdata->log_id) {
            return -1;
        } else if(a_node->logdata->log_id == a_other->logdata->log_id) {
            return 0;
        } else if(a_node->logdata->log_id > a_other->logdata->log_id) {
            return 1;
        }
    } else if(a_node->logdata->log_rdtsc >  a_other->logdata->log_rdtsc) {
        return 1;
    }
    return 0;
}


int64_t log_count()
{
    return fastlog_atomic64_read(&logdata_count);
}


void logdata_rbtree__init()
{
    logdata_rbtree_new(&logdata_rbtree);
    fastlog_atomic64_init(&logdata_count);
}
void logdata_rbtree__destroy(void (*cb)(struct logdata_decode *node, void *arg), void *arg)
{
    assert(cb && "NULL callback error.");
    
    logdata_rbtree_destroy(&logdata_rbtree, cb, arg);
}

void logdata_rbtree__insert(struct logdata_decode *new_node)
{
    logdata_rbtree_insert(&logdata_rbtree, new_node);
    fastlog_atomic64_inc(&logdata_count);
}

void logdata_rbtree__remove(struct logdata_decode *new_node)
{
    logdata_rbtree_remove(&logdata_rbtree, new_node);
    fastlog_atomic64_dec(&logdata_count);
}

struct logdata_decode * logdata_rbtree__search(int log_id, uint64_t log_rdtsc)
{
    fastlog_logdata_t log;
    log.log_id = log_id;
    log.log_rdtsc = log_rdtsc;
    
    struct logdata_decode logdata = {.logdata = &log};
    
    return logdata_rbtree_search(&logdata_rbtree, (const struct logdata_decode*)&logdata);
}


void logdata_rbtree__iter(void (*cb)(struct logdata_decode *meta, void *arg), void *arg)
{
    assert(cb && "NULL callback error.");
    
    struct logdata_decode *_next = NULL;

    for(_next = logdata_rbtree_first(&logdata_rbtree); 
        _next; 
        _next = logdata_rbtree_next(&logdata_rbtree, _next)) {
        cb(_next, arg);
    }
}


rb_gen(static _unused, logdata_rbtree_, logdata_rbtree_t, struct logdata_decode, rb_link_node_rdtsc, logdata_rbtree__cmp);



/**
 *  日志搜索 红黑树 函数
 *
 *  rb_proto:  日志搜索 红黑树 函数声明
 *  rb_gen:  日志搜索 红黑树 函数定义
 *
 *  log_search_rbtree__cmp: metadata_decode 结构对比
 *
 *  log_search_rbtree__init:  红黑树初始化 
 *  log_search_rbtree__destroy:   红黑树销毁
 *  log_search_rbtree__insert:  红黑树插入
 *  log_search_rbtree__remove:  红黑树删除
 *  log_search_rbtree__search:   红黑树查找
 *  log_search_rbtree__iter:  红黑树遍历 
 */
typedef rb_tree(struct log_search) log_search_rbtree_t;

static log_search_rbtree_t log_search_rbtrees[__LOG__RANGE_FILTER_NUM];

rb_proto(static, log_search_rbtree_, log_search_rbtree_t, struct log_search);

static int log_search_rbtree__cmp(const struct log_search *a_node, const struct log_search *a_other)
{

    return strcmp(a_node->string, a_other->string);
}



void log_search_rbtree__init()
{
    int i;
    for(i=0; i< __LOG__RANGE_FILTER_NUM; i++) {
        log_search_rbtree_new(&log_search_rbtrees[i]);
    }
}
void log_search_rbtree__destroy(LOG__RANGE_FILTER_ENUM type, void (*cb)(struct log_search *node, void *arg), void *arg)
{
    assert(cb && "NULL callback error.");
    
    log_search_rbtree_destroy(&log_search_rbtrees[type], cb, arg);
}

void log_search_rbtree__insert(LOG__RANGE_FILTER_ENUM type, struct log_search *new_node)
{
    log_search_rbtree_insert(&log_search_rbtrees[type], new_node);
}

void log_search_rbtree__remove(LOG__RANGE_FILTER_ENUM type, struct log_search *new_node)
{
    log_search_rbtree_remove(&log_search_rbtrees[type], new_node);
}

struct log_search * log_search_rbtree__search(LOG__RANGE_FILTER_ENUM type, char *string)
{
    struct log_search logdata = {.string = string};
    
    return log_search_rbtree_search(&log_search_rbtrees[type], (const struct log_search*)&logdata);
}

struct log_search *log_search_rbtree__search_or_create(LOG__RANGE_FILTER_ENUM type, char *string) 
{
    struct log_search *search = log_search_rbtree__search(type, string);
    if(!search) {
        struct log_search *newnode = (struct log_search *)malloc(sizeof(struct log_search));
        assert(newnode && "Malloc failed");

        newnode->string = string;
        newnode->string_type = type;
        newnode->log_cnt = 0;

        list_init(&newnode->log_list_head);

        log_search_rbtree__insert(type, newnode);

        //printf("func rbtree node - %s\n", newnode->string);
        
        search = newnode;
    }

    return search;
}


void log_search_rbtree__iter(LOG__RANGE_FILTER_ENUM type, void (*cb)(struct log_search *meta, void *arg), void *arg)
{
    assert(cb && "NULL callback error.");
    
    struct log_search *_next = NULL;

    for(_next = log_search_rbtree_first(&log_search_rbtrees[type]); 
        _next; 
        _next = log_search_rbtree_next(&log_search_rbtrees[type], _next)) {
        cb(_next, arg);
    }
}


rb_gen(static _unused, log_search_rbtree_, log_search_rbtree_t, struct log_search, rb_link, log_search_rbtree__cmp);

