/**
 *  FastLog 低时延 LOG日志
 *
 *  
 */
#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>

#include <sys/eventfd.h> //eventfd
#include <sys/epoll.h>

#include <fastlog.h>
#include <fastlog_list.h>
#include <fastlog_cycles.h>
#include <fastlog_staging_buffer.h>



#ifndef _FASTLOG_USE_EVENTFD
#error "Must define _FASTLOG_USE_EVENTFD 0 or 1."
/**
 * 此宏定义标识了一个代码分支，该分支决定用户线程和后台线程的交互方式
 *  
 *  _FASTLOG_USE_EVENTFD=0(false) 使用轮询的方式
 *  _FASTLOG_USE_EVENTFD=1(true) 使用通知(`eventfd()`)+轮询的方式
 *
 *  见`bg_event_epoll_fd`,`stagingBuffer_event_fd`,...
 *
 *  荣涛 2021年6月28日
 */
#define _FASTLOG_USE_EVENTFD 0
#endif



/**
 *  日志索引 
 *
 *  初始值在`fastlog_init()` 设置为 1
 *  解析命令中`parse_logdata` 遇到0值(或文件结尾)即为结束标志
 *
 */
static fastlog_atomic64_t  maxlogId;

/**
 *  默认日志文件大小和文件数
 */
static size_t max_nr_logfile = 24;
static size_t log_file_size = FATSLOG_LOG_FILE_SIZE_DEFAULT; 


/**
 *  后台线程和用户线程之间的缓冲区定义
 */
__thread struct StagingBuffer *stagingBuffer = NULL; 

struct StagingBuffer *threadBuffers[1024] = {NULL}; //最大支持的线程数，最多有多少个`stagingBuffer`
fastlog_atomic64_t  stagingBufferId;

#if _FASTLOG_USE_EVENTFD
static int bg_event_epoll_fd = -1;
static int bg_new_thread_event_fd = -1;
struct StagingBuffer *evt_fd_to_buffer[FD_SETSIZE] = {NULL};
__thread int stagingBuffer_event_fd = -1;
#endif

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
static uint64_t program_cycles_per_sec = 0;
static uint64_t program_start_rdtsc = 0;
static time_t   program_unix_time_sec = 0;      //time(2)
static struct utsname program_unix_uname = {{0}};   //uname(2)

enum FASTLOG_LEVEL __curr_level = FASTLOG_DEBUG;


static void mmap_new_fastlog_file(struct fastlog_file_mmap *mmap_file, 
                                    char *filename, 
                                    char *backupfilename, 
                                    size_t size,
                                    int magic,
                                    uint64_t cycles_per_sec,
                                    uint64_t rdtsc,
                                    time_t time_from_19700101,
                                    struct utsname *unix_uname,
                                    char **mmap_curr_ptr);


/* 后台程序处理一条log的 arg 处理函数 */
static fl_inline void bg_handle_one_log(struct arg_hdr *log_args, size_t size)
{
    if(unlikely(log_args->log_id < 0)) {
        printf("LOGID %d < 0\n", log_args->log_id);
        assert(0);
    }

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
        if(file_id > max_nr_logfile) {
            file_id = 0;
        }
        
        //日志数据文件映射
        mmap_new_fastlog_file(&logdata_mmap, 
                            new_log_filename, 
                            new_log_filename_old, 
                            log_file_size, 
                            FATSLOG_LOG_HEADER_MAGIC_NUMBER, 
                            program_cycles_per_sec, 
                            program_start_rdtsc, 
                            program_unix_time_sec,
                            &program_unix_uname,
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
    int _unused i;
    struct arg_hdr *arghdr = NULL;
    struct StagingBuffer *curr_buffer = NULL;


#if _FASTLOG_USE_EVENTFD
    int _unused nfds = 0;
    int _unused curr_event_fd = -1;
    struct epoll_event _unused events[32] = { };
    eventfd_t _evt_value;
#endif

    /**
     *  后台线程的优先级稍低
     */
    nice(10);

    /**
     *  限制 CPU 利用率
     *
     *  有以下几个方案
     *  -------------------------------------------------------------------
     *  (1)使用 cgroup 将会影响该进程其他线程的CPU使用
     *      ```bash
     *      # 创建 cgroup
     *      cgcreate -g cpu:fastlog
     *      # 限制 CPU 利用率 为 30%(需要登录 root 用户, sudo 也不行)
     *      echo 30000 >  /sys/fs/cgroup/cpu/fastlog/cpu.cfs_quota_us
     *      # 将后台线程添加至 CPU 控制组
     *      cgclassify -g cpu:fastlog [pid]
     *      ```
     *    发现使用 cgroup 会影响到其他线程，即整个进程的CPU被限制为 30%
     *
     *  -------------------------------------------------------------------
     *  (2)使用 setrlimit 
     */

    /**
     *  后台线程主任务
     */
    while(1) {

#if _FASTLOG_USE_EVENTFD
        /**
         *  当使用 通知+轮询 方式(epoll+eventfd)
         */
        nfds = epoll_wait(bg_event_epoll_fd, events, 32, -1);
        
        if(nfds <= 0) {
            fprintf(stderr, "epoll_wait return -> %d, error(%d): %s\n", nfds, errno, strerror(errno));
            assert(0);
        }
        
        for(;nfds--;)
            
#else//_FASTLOG_USE_EVENTFD

        /**
         *  当使用 轮询 方式
         *  遍历和所有线程之间的 staging buffer
         */
        for(i=0; i<fastlog_atomic64_read(&stagingBufferId); i++)
#endif//_FASTLOG_USE_EVENTFD
        {

#if _FASTLOG_USE_EVENTFD
            /**
             *  
             */
            curr_event_fd = events[nfds].data.fd;
            if(unlikely(bg_new_thread_event_fd == curr_event_fd)) {
                eventfd_read(bg_new_thread_event_fd, &_evt_value);
                continue;
            }
            curr_buffer = evt_fd_to_buffer[curr_event_fd];
            eventfd_read(curr_event_fd, &_evt_value);
            //printf("eventfd read = %ld\n", _evt_value);
            //下面为我测试的输出：
            //eventfd read = 13
            //eventfd read = 11
            //eventfd read = 13
            //eventfd read = 68
            //eventfd read = 13
            //eventfd read = 11
            //eventfd read = 9
            //eventfd read = 9
            //eventfd read = 61
            //eventfd read = 9
            
#else//_FASTLOG_USE_EVENTFD

            curr_buffer = threadBuffers[i];

#endif//_FASTLOG_USE_EVENTFD

#if _FASTLOG_USE_EVENTFD
peek_buffer_again:
#endif//_FASTLOG_USE_EVENTFD
            
            /* 从 staging buffer 中获取一块内存 */
            arghdr = (struct arg_hdr*)peek_buffer(curr_buffer, &size);
            
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
#if _FASTLOG_USE_EVENTFD
                _evt_value --;
#endif//_FASTLOG_USE_EVENTFD
                /* 确认消费 */
                consume_done(curr_buffer, __size);
            
                char *p = (char *)arghdr;

                p += __size;
                remain -= __size;
                arghdr = (struct arg_hdr*)p;
            }
#if _FASTLOG_USE_EVENTFD
            /**
             *  当实际读取的 日志 数量小于 eventfd 中通知的数量，需要再次解析。
             *  如果出错，这里的`_evt_value`恒大于0
             */
            if(_evt_value>0) {
                goto peek_buffer_again;
            }
#else
            //usleep(1);       
#endif//_FASTLOG_USE_EVENTFD

        }
    }

    return NULL;
}

/**
 *  获取未使用的 log_id
 *  单纯的原子操作分为两步，依旧不安全，所以使用自旋锁。
 */
static inline int __get_unused_logid()
{
    static pthread_spinlock_t spin = 1;
    int log_id;
    
    pthread_spin_lock(&spin);

    log_id = fastlog_atomic64_read(&maxlogId);
    fastlog_atomic64_inc(&maxlogId);
    
    pthread_spin_unlock(&spin);
    
    return log_id;
}

void fastlog_exit()
{
    //printf("FastLog exit.\n");
    //pthread_kill(fastlog_background_thread, SIGINT);
    
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
                time_t time_from_19700101,
                struct utsname *unix_uname,
                char **mmap_curr_ptr)
{
    int _unused ret = -1;
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
    meta_hdr->unix_time_sec = time_from_19700101;
    memcpy(&meta_hdr->unix_uname, unix_uname, sizeof(struct utsname));
    
    
    *mmap_curr_ptr += sizeof(struct fastlog_file_header);
    msync(mmap_file->mmapaddr, sizeof(struct fastlog_file_header), MS_ASYNC);
    
}


void fastlog_setlevel(enum FASTLOG_LEVEL level)
{
    __curr_level = level;
}
enum FASTLOG_LEVEL fastlog_getlevel()
{
    return __curr_level;
}

#if _FASTLOG_USE_EVENTFD
static int __bg_new_event_add_to_epoll()
{
    int ret = -1;
    
    struct epoll_event event;
    
    int evt_fd = eventfd(0, EFD_CLOEXEC);
    assert(evt_fd != -1 && "eventfd(EFD_CLOEXEC) failed.");
    
    event.data.fd = evt_fd;
    event.events = EPOLLIN; //必须采用水平触发
    ret = epoll_ctl(bg_event_epoll_fd, EPOLL_CTL_ADD, event.data.fd, &event);
    assert(ret == 0 && "epoll_ctl(EPOLL_CTL_ADD) failed.\n");
    //printf("add %d to epoll fd %d\n", evt_fd, bg_event_epoll_fd);

    return evt_fd;
}
#endif


static void __bg_init_event_epoll()
{
#if _FASTLOG_USE_EVENTFD

    bg_event_epoll_fd = epoll_create(1);
    assert(bg_event_epoll_fd && "Epoll create error");

    bg_new_thread_event_fd = __bg_new_event_add_to_epoll();
    //printf("bg_new_thread_event_fd = %d\n", bg_new_thread_event_fd);
    
#endif    
}

int __bg_add_buffer_event_to_epoll()
{
#if _FASTLOG_USE_EVENTFD

    if(stagingBuffer_event_fd != -1) {
        //printf("Already create eventfd = %d\n", stagingBuffer_event_fd);
        return -1;
    }
    stagingBuffer_event_fd = __bg_new_event_add_to_epoll();
    //printf("stagingBuffer_event_fd = %d\n", stagingBuffer_event_fd);
    
    evt_fd_to_buffer[stagingBuffer_event_fd] = stagingBuffer;

    eventfd_write(bg_new_thread_event_fd, 1);
#endif    

    return 0;
}


//__attribute__((constructor(105))) 
void fastlog_init(enum FASTLOG_LEVEL level, size_t nr_logfile, size_t logfile_size)
{
    int _unused ret = -1;
    //printf("FastLog initial.\n");

    //list_init(&__fastlog_level_list);

    fastlog_setlevel(level);
    
    /* 日志大小 */
    if(nr_logfile)
        max_nr_logfile = nr_logfile;
    if(logfile_size)
        log_file_size = logfile_size;

    pthread_spin_init(&metadata_mmap_lock, PTHREAD_PROCESS_PRIVATE);

    __fastlog_cycles_init();

    program_cycles_per_sec = __fastlog_get_cycles_per_sec();
    program_start_rdtsc = __fastlog_rdtsc();
    program_unix_time_sec = time(NULL);
    uname(&program_unix_uname);

    //printf("Cycles/s        = %ld.\n", program_cycles_per_sec);
    //printf("Start Cycles    = %ld.\n", program_start_rdtsc);

    //元数据文件映射
    mmap_new_fastlog_file(&metadata_mmap, 
                          FATSLOG_METADATA_FILE_DEFAULT, 
                          FATSLOG_METADATA_FILE_DEFAULT".old", 
                          FATSLOG_METADATA_FILE_SIZE_DEFAULT, 
                          FATSLOG_METADATA_HEADER_MAGIC_NUMBER, 
                          program_cycles_per_sec, 
                          program_start_rdtsc, 
                          program_unix_time_sec,
                          &program_unix_uname,
                          &metadata_mmap_curr_ptr);

    
    //日志数据文件映射
    mmap_new_fastlog_file(&logdata_mmap, 
                          FATSLOG_LOG_FILE_DEFAULT, 
                          FATSLOG_LOG_FILE_DEFAULT".old", 
                          log_file_size, 
                          FATSLOG_LOG_HEADER_MAGIC_NUMBER, 
                          program_cycles_per_sec, 
                          program_start_rdtsc, 
                          program_unix_time_sec,
                          &program_unix_uname,
                          &logdata_mmap_curr_ptr);
    

    fastlog_atomic64_init(&stagingBufferId);
    fastlog_atomic64_init(&maxlogId);
    fastlog_atomic64_inc(&maxlogId);    //logID 初始值 1

    /**
     *  创建 epoll FD
     */
    __bg_init_event_epoll();

    /**
     *  启动后台线程，用于和用户线程之间进行数据交互，写文件映射内存。
     */
    if(0 != pthread_create(&fastlog_background_thread, NULL, bg_task_routine, NULL)) {
        fprintf(stderr, "create fastlog background thread failed.\n");
        assert(0);
    }
    pthread_setname_np(fastlog_background_thread, "FLbgd");

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
//    string_buf[0] = '\0';
    
    *thread_name = string_buf;
    string_buf += metadata->thread_name_len;

    
    
    return curr_metadata_len;
}

/* 保存 元数据 */
static void save_fastlog_metadata(int log_id, int level, const char *name, const char *file, const char *func, int line, const char *format)
{
    int ret = -1;
    char thread_name[32];
    
    ret = pthread_getname_np(pthread_self(), thread_name, 32);
    if (ret != 0) {
        strncpy(thread_name, "unknown", 32);
    }

    //不要路径
    const char *base_file = basename((char*)file);
    
    pthread_spin_lock(&metadata_mmap_lock);

    //    printf("log_id = %d\n", log_id);
    struct fastlog_metadata *metadata = (struct fastlog_metadata *)metadata_mmap_curr_ptr;

    metadata->magic = FATSLOG_METADATA_MAGIC_NUMBER;
    metadata->log_id = log_id;
    metadata->log_level = level;
    metadata->log_line = line;
    metadata->user_string_len  = strlen(name) + 1;
    metadata->src_filename_len = strlen(base_file) + 1;
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
    memcpy(string_buf, base_file, metadata->src_filename_len);
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
int __fastlog_get_unused_logid(int level, const char *name, const char *file, const char *func, int line, const char *format)
{
    int log_id = __get_unused_logid();

    save_fastlog_metadata(log_id, level, name, file, func, line, format);
    
    return log_id; 
}


