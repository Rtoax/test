#include <fastlog_decode.h>

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


