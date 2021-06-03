/**
 *  FastLog 低时延 LOG日志
 *
 *  代码日志
 *  2021年5月21日    创建源码文件
 *  2021年6月2日 
 *  2021年6月3日 rdtsc部分；log_id获取；
 *
 */
#include <pthread.h>
#include <fastlog.h>


static pthread_spinlock_t __fastlog_logid_lock;

static inline int __get_unused_logid()
{
    int log_id;
    static int prev_log_id = 0;

    pthread_spin_lock(&__fastlog_logid_lock);
    log_id = prev_log_id++;
    pthread_spin_unlock(&__fastlog_logid_lock);

    return log_id;
}

static int __attribute__((constructor(105))) __fastlog_init()
{
    __fastlog_cycles_init();

    pthread_spin_init(&__fastlog_logid_lock, PTHREAD_PROCESS_PRIVATE);
}


// 1. 生成 logid;
// 2. 组件 logid 对应的元数据(数据结构)
// 3. 保存元数据
int __fastlog_get_unused_logid(int level, const char *name, char *file, char *func, int line, const char *format)
{
    int log_id = __get_unused_logid();

    printf("log_id = %d\n", log_id);

    return log_id; 
}


