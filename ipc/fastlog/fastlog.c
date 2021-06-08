/**
 *  FastLog 低时延 LOG日志
 *
 *  
 */
#include <pthread.h>
#include <fastlog.h>


__thread struct StagingBuffer *stagingBuffer = NULL; 
struct StagingBuffer *threadBuffers[1024] = {NULL}; //最大支持的线程数，最多有多少个`stagingBuffer`
fastlog_atomic64_t  stagingBufferId;
fastlog_atomic64_t  maxlogId;

/* 后台处理线程 */
static pthread_t fastlog_background_thread = 0;


/* 后台程序处理一条log的 arg 处理函数 */
static inline void bg_handle_one_log(struct arg_hdr *log_args, size_t size)
{
//    printf("logId = %d, rdtsc = %ld, args size = %d\n", log_args->log_id, log_args->log_rdtsc, size);
//    char buffer[1024];
//    memcpy(buffer, log_args, size);
}

/* 后台进程主任务回调函数 */
static void * bg_task_routine(void *arg)
{
    size_t size, remain, __size;
    int i;
    struct arg_hdr *arghdr = NULL;
    
    while(1) {
        for(i=0; i<fastlog_atomic64_read(&stagingBufferId); i++) {
            
            arghdr = (struct arg_hdr*)peek_buffer(threadBuffers[i], &size);
            
            if(!arghdr || size == 0) {
                continue;
            }
            remain = size;
            while(remain > 0) {
                __size = arghdr->log_args_size + sizeof(struct arg_hdr);
                bg_handle_one_log(arghdr, __size);
                consume_done(threadBuffers[i], __size);
            
                char *p = (char *)arghdr;

                p += __size;
                remain -= __size;
                arghdr = (struct arg_hdr*)p;
            }
        }
    }
}

static inline int __get_unused_logid()
{
    int log_id = fastlog_atomic64_read(&maxlogId);
    fastlog_atomic64_inc(&maxlogId);

    return log_id;
}

static void __attribute__((constructor(105))) __fastlog_init()
{
    __fastlog_cycles_init();

    fastlog_atomic64_init(&stagingBufferId);
    fastlog_atomic64_init(&maxlogId);

    if(0 != pthread_create(&fastlog_background_thread, NULL, bg_task_routine, NULL)) {
        fprintf(stderr, "create fastlog background thread failed.\n");
        assert(0);
    }
}

static void save_metadata(struct fastlog_log_metadata *metadata)
{
    printf(" %-3d %-6s %20s: %-d\n", 
            metadata->log_id, 
            FASTLOG_LEVEL_NAME[metadata->log_level], 
            metadata->src_file,
            metadata->log_line);

    
            
}

// 1. 生成 logid;
// 2. 组件 logid 对应的元数据(数据结构)
// 3. 保存元数据
int __fastlog_get_unused_logid(int level, const char *name, char *file, char *func, int line, const char *format)
{
    int log_id = __get_unused_logid();

//    printf("log_id = %d\n", log_id);
    struct fastlog_log_metadata metadata = {
        .log_id = log_id,
        .log_level = level,
        .log_line = line,
        .metadata_size = 0,
        .src_file = file,
//        .src_private_name,
        .print_format = format,
    };

    save_metadata(&metadata);
    
    return log_id; 
}


