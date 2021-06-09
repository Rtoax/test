/**
 *  FastLog 低时延 LOG日志
 *
 *  
 */
#include <pthread.h>
#include <fastlog.h>
#include <sys/mman.h>


/**
 *  日志索引
 */
static fastlog_atomic64_t  maxlogId;


/**
 *  后台线程和用户线程之间的缓冲区定义
 */
__thread struct StagingBuffer *stagingBuffer = NULL; 
struct StagingBuffer *threadBuffers[1024] = {NULL}; //最大支持的线程数，最多有多少个`stagingBuffer`
fastlog_atomic64_t  stagingBufferId;

/* 后台处理线程 */
static pthread_t fastlog_background_thread = 0;

/**
 *  内存映射
 */
static struct fastlog_file_mmap metadata_mmap, logdata_mmap;

/**
 *  保存文件映射的当前指针位置
 *
 *  需要注意:
 *  元数据指针`metadata_mmap_curr_ptr`会被多线程访问，使用`metadata_mmap_lock`保护
 *  日志数据指针`logdata_mmap_curr_ptr`只会被`fastlog_background_thread`访问，故无需锁
 *  
 */
static pthread_spinlock_t metadata_mmap_lock;
static char *metadata_mmap_curr_ptr = NULL;
static char *logdata_mmap_curr_ptr = NULL;

/**
 *  程序初始化时被初始化，程序运行过程中为只读常数
 */
static uint64_t program_cycles_per_sec;
static uint64_t program_start_rdtsc;


static void mmap_new_fastlog_file(struct fastlog_file_mmap *mmap_file, 
                                    char *filename, 
                                    char *backupfilename, 
                                    size_t size,
                                    int magic,
                                    uint64_t cycles_per_sec,
                                    uint64_t rdtsc,
                                    char **mmap_curr_ptr);


/* 后台程序处理一条log的 arg 处理函数 */
static inline void bg_handle_one_log(struct arg_hdr *log_args, size_t size)
{
    /**
     *  当日志映射文件已满
     *  
     *  当内存映射`当前指针`大于文件映射地址+文件长度后，将这个文件同步至磁盘（元数据也进行了同步）
     *  然后，新建一个文件映射，注意这个文件映射规律如下：
     *      fastlog.log     [默认名称]
     *      fastlog.log.0   [上面文件满后，新建文件，并映射到内存]
     *      fastlog.log.1   [以此类推]
     *      fastlog.log.[N] [参数`N`可配置]
     *  
     */
    if(unlikely(logdata_mmap_curr_ptr + size > (logdata_mmap.mmapaddr + logdata_mmap.mmap_size))) {

        /**
         *  将其同步至磁盘 
         *  
         *  这里的开销是比较大的，可以释放增大每个映射文件的大小来减少同步次数
         *  
         */
        msync_fastlog_logfile_write(&metadata_mmap);
        msync_fastlog_logfile_write(&logdata_mmap);

        /* 取消映射 */
        unmmap_fastlog_logfile(&logdata_mmap);

        /* 创建新的映射 */
        static int file_id = 0;
        char new_log_filename[256] = {0};
        char new_log_filename_old[256] = {0};
        
        snprintf(new_log_filename, 256, "%s.%d", FATSLOG_LOG_FILE_DEFAULT, file_id);
        snprintf(new_log_filename_old, 256, "%s.%d.old", FATSLOG_LOG_FILE_DEFAULT, file_id);
        file_id ++;
        
        //日志数据文件映射
        mmap_new_fastlog_file(&logdata_mmap, 
                            new_log_filename, 
                            new_log_filename_old, 
                            FATSLOG_LOG_FILE_SIZE_DEFAULT, 
                            FATSLOG_LOG_HEADER_MAGIC_NUMBER, 
                            program_cycles_per_sec, 
                            program_start_rdtsc, 
                            &logdata_mmap_curr_ptr);
    }

    /**
     *  保存单条日志 的 argument 数据
     */
    memcpy(logdata_mmap_curr_ptr, log_args, size);
    logdata_mmap_curr_ptr += size;
    
}


/* 后台进程主任务回调函数 */
static void * bg_task_routine(void *arg)
{
    size_t size, remain, __size;
    int i;
    struct arg_hdr *arghdr = NULL;

    /**
     *  后台线程主任务
     */
    while(1) {

        /* 遍历和所有线程之间的 staging buffer */
        for(i=0; i<fastlog_atomic64_read(&stagingBufferId); i++) {

            /* 从 staging buffer 中获取一块内存 */
            arghdr = (struct arg_hdr*)peek_buffer(threadBuffers[i], &size);
            
            if(!arghdr || size == 0) {
                continue;
            }

            /**
             *  遍历上面获取的这块内存，并将每条消息使用消息处理函数处理
             */
            remain = size;
            while(remain > 0) {
                __size = arghdr->log_args_size + sizeof(struct arg_hdr);

                bg_handle_one_log(arghdr, __size);

                /* 确认消费 */
                consume_done(threadBuffers[i], __size);
            
                char *p = (char *)arghdr;

                p += __size;
                remain -= __size;
                arghdr = (struct arg_hdr*)p;
            }
        }
    }
}

/**
 *  
 */
static inline int __get_unused_logid()
{
    int log_id = fastlog_atomic64_read(&maxlogId);
    fastlog_atomic64_inc(&maxlogId);

    return log_id;
}

//__attribute__((destructor(105)))
void fastlog_exit()
{
    printf("FastLog exit.\n");
    unmmap_fastlog_logfile(&metadata_mmap);
    unmmap_fastlog_logfile(&logdata_mmap);
}

/**
 *  映射文件
 */
static void mmap_new_fastlog_file(struct fastlog_file_mmap *mmap_file, 
                char *filename, 
                char *backupfilename, 
                size_t size,
                int magic,
                uint64_t cycles_per_sec,
                uint64_t rdtsc,
                char **mmap_curr_ptr)
{
    int ret = -1;
    //元数据文件映射
    ret = mmap_fastlog_logfile_write(mmap_file,
                                     filename, 
                                     backupfilename, 
                                     size);
    
    memset(mmap_file->mmapaddr, 0x00, mmap_file->mmap_size);
    *mmap_curr_ptr = mmap_file->mmapaddr;

    struct fastlog_file_header *meta_hdr = 
                (struct fastlog_file_header *)(*mmap_curr_ptr);

    meta_hdr->magic = magic;
    meta_hdr->cycles_per_sec = cycles_per_sec;
    meta_hdr->start_rdtsc = rdtsc;

    *mmap_curr_ptr += sizeof(struct fastlog_file_header);
    msync(mmap_file->mmapaddr, sizeof(struct fastlog_file_header), MS_ASYNC);
    
}


//__attribute__((constructor(105))) 
void fastlog_init()
{
    int ret = -1;
    printf("FastLog initial.\n");

    pthread_spin_init(&metadata_mmap_lock, PTHREAD_PROCESS_PRIVATE);

    __fastlog_cycles_init();

    program_cycles_per_sec = __fastlog_get_cycles_per_sec();
    program_start_rdtsc = __fastlog_rdtsc();

    //元数据文件映射
    mmap_new_fastlog_file(&metadata_mmap, 
                          FATSLOG_METADATA_FILE_DEFAULT, 
                          FATSLOG_METADATA_FILE_DEFAULT".old", 
                          FATSLOG_METADATA_FILE_SIZE_DEFAULT, 
                          FATSLOG_METADATA_HEADER_MAGIC_NUMBER, 
                          program_cycles_per_sec, 
                          program_start_rdtsc, 
                          &metadata_mmap_curr_ptr);

    
    //日志数据文件映射
    mmap_new_fastlog_file(&logdata_mmap, 
                          FATSLOG_LOG_FILE_DEFAULT, 
                          FATSLOG_LOG_FILE_DEFAULT".old", 
                          FATSLOG_LOG_FILE_SIZE_DEFAULT, 
                          FATSLOG_LOG_HEADER_MAGIC_NUMBER, 
                          program_cycles_per_sec, 
                          program_start_rdtsc, 
                          &logdata_mmap_curr_ptr);
    

    fastlog_atomic64_init(&stagingBufferId);
    fastlog_atomic64_init(&maxlogId);

    /**
     *  启动后台线程，用于和用户线程之间进行数据交互，写文件映射内存。
     */
    if(0 != pthread_create(&fastlog_background_thread, NULL, bg_task_routine, NULL)) {
        fprintf(stderr, "create fastlog background thread failed.\n");
        assert(0);
    }
}


int parse_fastlog_logdata(fastlog_logdata_t *logdata, int *log_id, int *args_size, uint64_t *rdtsc, char **argsbuf)
{
    *log_id = logdata->log_id;
    *args_size = logdata->log_args_size;
    *rdtsc = logdata->log_rdtsc;
    *argsbuf = logdata->log_args_buff;

    return *args_size + sizeof(struct arg_hdr);
}

int parse_fastlog_metadata(struct fastlog_metadata *metadata,
                int *log_id, int *level, char **name, 
                char **file, char **func, int *line, char **format, char **thread_name)
{
    int curr_metadata_len = metadata->metadata_size;
    
    char *string_buf = metadata->string_buf;
    
    *log_id = metadata->log_id;
    *level = metadata->log_level;
    *line = metadata->log_line;

    *name = string_buf;
    string_buf += metadata->user_string_len;

    *file = string_buf;
    string_buf += metadata->src_filename_len;
    
    *func = string_buf;
    string_buf += metadata->src_function_len;
    
    *format = string_buf;
    string_buf += metadata->print_format_len;
    
    *thread_name = string_buf;
    string_buf += metadata->thread_name_len;

    
    
    return curr_metadata_len;
}

/* 保存 元数据 */
static void save_fastlog_metadata(int log_id, int level, char *name, char *file, char *func, int line, char *format)
{
    int ret = -1;
    char thread_name[32];
    
    ret = pthread_getname_np(pthread_self(), thread_name, 32);
    if (ret != 0) {
        strncpy(thread_name, "unknown", 32);
    }
    
    pthread_spin_lock(&metadata_mmap_lock);

    //    printf("log_id = %d\n", log_id);
    struct fastlog_metadata *metadata = (struct fastlog_metadata *)metadata_mmap_curr_ptr;

    metadata->magic = FATSLOG_METADATA_MAGIC_NUMBER;
    metadata->log_id = log_id;
    metadata->log_level = level;
    metadata->log_line = line;
    metadata->user_string_len  = strlen(name) + 1;
    metadata->src_filename_len = strlen(file) + 1;
    metadata->src_function_len = strlen(func) + 1;
    metadata->print_format_len = strlen(format) + 1;
    metadata->thread_name_len = strlen(thread_name) + 1;


    metadata->metadata_size = sizeof(struct fastlog_metadata) 
                                + metadata->user_string_len 
                                + metadata->src_filename_len 
                                + metadata->src_function_len 
                                + metadata->print_format_len
                                + metadata->thread_name_len;

    char *string_buf = metadata->string_buf;

    memcpy(string_buf, name, metadata->user_string_len);
    string_buf += metadata->user_string_len;
    memcpy(string_buf, file, metadata->src_filename_len);
    string_buf += metadata->src_filename_len;
    memcpy(string_buf, func, metadata->src_function_len);
    string_buf += metadata->src_function_len;
    memcpy(string_buf, format, metadata->print_format_len);
    string_buf += metadata->print_format_len;
    memcpy(string_buf, thread_name, metadata->thread_name_len);
    string_buf += metadata->thread_name_len;
    

    metadata_mmap_curr_ptr += metadata->metadata_size;

    pthread_spin_unlock(&metadata_mmap_lock);
            
}


// 1. 生成 logid;
// 2. 组件 logid 对应的元数据(数据结构)
// 3. 保存元数据
int __fastlog_get_unused_logid(int level, char *name, char *file, char *func, int line, char *format)
{
    int log_id = __get_unused_logid();

    save_fastlog_metadata(log_id, level, name, file, func, line, format);
    
    return log_id; 
}


