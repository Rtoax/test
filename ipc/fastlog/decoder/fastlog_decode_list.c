#include <fastlog_decode.h>
#include <bitmask/bitmask.h>


static struct bitmask *log_id_bitmask = NULL;

#define MAX_LOG_ID  65535

void log_ids__init()
{
    log_id_bitmask = bitmask_alloc(MAX_LOG_ID);
    assert(log_id_bitmask && "log_id_bitmask init allocated failed.");
}
void log_ids__destroy()
{
    if(likely(log_id_bitmask)) {
        bitmask_free(log_id_bitmask);
        log_id_bitmask = NULL;
    }
}

void log_ids__set(int log_id)
{
    if(unlikely(!log_id_bitmask)) {
        assert(0 && "log_id_bitmask not init");
    }
    if(unlikely(log_id > MAX_LOG_ID)) {
        assert(0 && "log_id bigger than 65535");
    }

    bitmask_setbit(log_id_bitmask, log_id);
}

int log_ids__isset(int log_id)
{
    if(unlikely(!log_id_bitmask)) {
        assert(0 && "log_id_bitmask not init");
    }
    return bitmask_isbitset(log_id_bitmask, log_id);
}

int log_ids__first()
{
    if(unlikely(!log_id_bitmask)) {
        assert(0 && "log_id_bitmask not init");
    }
     return bitmask_first(log_id_bitmask);
}

int log_ids__next(int log_id)
{
    if(unlikely(!log_id_bitmask)) {
        assert(0 && "log_id_bitmask not init");
    }
    return bitmask_next(log_id_bitmask, log_id+1);
}

int log_ids__last()
{
    if(unlikely(!log_id_bitmask)) {
        assert(0 && "log_id_bitmask not init");
    }
     return bitmask_last(log_id_bitmask);
}


void log_ids__iter(void (*cb)(int log_id, void* arg), void *arg)
{
    if(unlikely(!log_id_bitmask) || unlikely(!cb)) {
        assert(0 && "log_id_bitmask not init or callback NULL.");
    }
    int log_id;
    for(log_id = log_ids__first(); log_ids__isset(log_id); log_id = log_ids__next(log_id)) {
        cb(log_id, arg);
    }

}

#ifdef TEST
void test_log_ids()
{
    int log_id;
    
    log_ids__init();

    log_ids__set(1);
    log_ids__set(6);
    log_ids__set(19);
    log_ids__set(11349);
    log_ids__set(129);

    void callback(int log_id, void *arg) {
        printf("log_id = %d\n", log_id);
    }
    log_ids__iter(callback, NULL);

    exit(1);
}
#endif


/**
 *  所有级别的链表
 *
 *  level_lists__init:  初始化级别链表
 *  
 *  level_list__insert: 向一个级别链表中插入
 *  level_list__remove: 从一个级别链表中移除
 *  level_list__iter:   遍历一个级别的链表
 */
static struct list level_lists[FASTLOGLEVELS_NUM];          /* 日志级别的链表 */
static fastlog_atomic64_t _level_count[FASTLOGLEVELS_NUM];   /* 日志级别统计 */


int64_t level_count(enum FASTLOG_LEVEL level)
{
    return fastlog_atomic64_read(&_level_count[level]);
}

int64_t level_count_all()
{
    int64_t count = 0;
    int i;
    for(i=0; i<FASTLOGLEVELS_NUM; i++) {
        count += fastlog_atomic64_read(&_level_count[i]);
    }
    return count;
}


void level_lists__init()
{
    int i;
    for(i=0; i<FASTLOGLEVELS_NUM; i++) {
        list_init(&level_lists[i]);
        fastlog_atomic64_init(&_level_count[i]);
    }
}

void level_list__insert(enum FASTLOG_LEVEL level, struct logdata_decode *logdata)
{
    if(unlikely(level >= FASTLOGLEVELS_NUM)) {
        assert(0 && "wrong level in" && __func__);
    }
    list_insert(&level_lists[level], &logdata->list_level);
    fastlog_atomic64_inc(&_level_count[level]);
}

void level_list__remove(enum FASTLOG_LEVEL level, struct logdata_decode *logdata)
{
    if(unlikely(level >= FASTLOGLEVELS_NUM)) {
        assert(0 && "wrong level in" && __func__);
    }
    list_remove(&logdata->list_level);
    fastlog_atomic64_dec(&_level_count[level]);
}

void level_list__iter(enum FASTLOG_LEVEL level, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg)
{
    if(unlikely(level >= FASTLOGLEVELS_NUM)) {
        assert(0 && "wrong level in" && __func__);
    }
    assert(cb && "NULL callback error.");
    
    struct logdata_decode *logdata;
    list_for_each_entry(logdata, &level_lists[level], list_level) {
        cb(logdata, arg);
    }
}


/**
 *  每个元数据中，都存有一个 log_id 的链表
 */
void id_lists__init_raw(struct metadata_decode *metadata)
{
    if(unlikely(!metadata)) {
        assert(0 && "NULL error" && __func__);
    }
    list_init(&metadata->id_list);
    metadata->id_cnt = 0;
}

void id_lists__init(int log_id)
{
    struct metadata_decode *metadata = metadata_rbtree__search(log_id);

    id_lists__init_raw(metadata);
}

void id_list__insert_raw(struct metadata_decode *metadata, struct logdata_decode *logdata)
{
    if(unlikely(!metadata) || unlikely(!logdata)) {
        assert(0 && "NULL error" && __func__);
    }
    list_insert(&metadata->id_list, &logdata->list_id);
    metadata->id_cnt ++;
}


void id_list__insert(int log_id, struct logdata_decode *logdata)
{
    struct metadata_decode *metadata = metadata_rbtree__search(log_id);
    
    id_list__insert_raw(metadata, logdata);
}

void id_list__remove_raw(struct metadata_decode *metadata, struct logdata_decode *logdata)
{   
    if(unlikely(!metadata) || unlikely(!logdata)) {
        assert(0 && "NULL error" && __func__);
    }
    list_remove(&logdata->list_id);
    metadata->id_cnt --;
}


void id_list__remove(int log_id, struct logdata_decode *logdata)
{
    struct metadata_decode *metadata = metadata_rbtree__search(log_id);
    
    id_list__remove_raw(metadata, logdata);
}

void id_list__iter_raw(struct metadata_decode *metadata, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg)
{
    if(unlikely(!metadata)) {
        assert(0 && "NULL error" && __func__);
    }
    assert(cb && "NULL callback error.");
    
    struct logdata_decode *logdata;
    list_for_each_entry(logdata, &metadata->id_list, list_id) {
        cb(logdata, arg);
    }
}
void id_list__iter(int log_id, void (*cb)(struct logdata_decode *logdata, void *arg), void *arg)
{
    struct metadata_decode *metadata = metadata_rbtree__search(log_id);
    
    id_list__iter_raw(metadata, cb, arg);
}

