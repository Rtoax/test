/**
 *  FastLog 低时延 LOG日志 解析工具
 *
 *  
 */
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <fastlog.h>

/* 解析出的数据 */
struct metadata_decode {
    struct fastlog_metadata *metadata;
    char* user_string; 
    char* src_filename; 
    char* src_function; 
    char* print_format;
    char* thread_name;
};


static struct fastlog_file_mmap ro_metadata_mmap, ro_logdata_mmap;

static struct fastlog_file_header *metadata_header = NULL;
static struct fastlog_file_header *logdata_header = NULL;


int parse_fastlog_metadata(struct fastlog_metadata *metadata,
                int *log_id, int *level, char **name,
                char **file, char **func, int *line, char **format, 
                char **thread_name);



int parse_fastlog_logdata(fastlog_logdata_t *logdata, int *log_id, int *args_size, uint64_t *rdtsc, char **argsbuf);


static int parse_metadata(struct fastlog_metadata *metadata)
{
    int ret;
    
    int log_id;
    int level;
    char *name;
    char *file;
    char *func;
    int line;
    char *format;
    char *thread_name;

parse_next:

    if(metadata->magic != FATSLOG_METADATA_MAGIC_NUMBER) {
        assert(0 && "wrong format of metadata file.\n");
    }

//    struct metadata_decode *decode_metadata = (struct metadata_decode*)malloc(sizeof(struct metadata_decode));
//    decode_metadata->metadata = metadata;
    
    ret = parse_fastlog_metadata(metadata, &log_id, &level, &name, &file, &func, &line, &format, &thread_name);

//    decode_metadata->user_string = name;
//    decode_metadata->src_filename = file;
//    decode_metadata->src_function = func;
//    decode_metadata->print_format = format;
//    decode_metadata->thread_name = thread_name;


    printf("log_id  = %d\n", log_id);
    printf("thread  = %s\n", thread_name);
    printf("level   = %d\n", level);
    printf("line    = %d\n", line);
    printf("name    = %s\n", name);
    printf("file    = %s\n", file);
    printf("func    = %s\n", func);
    printf("format  = %s\n", format);

    metadata = (((char*)metadata) + ret );

    if(metadata->magic == FATSLOG_METADATA_MAGIC_NUMBER) {
        goto parse_next;
    }


    return ret;
}


static int parse_logdata(fastlog_logdata_t *logdata)
{
    int ret;
    
    int log_id; //所属ID
    int args_size;
    uint64_t rdtsc;
    char *args_buff;


parse_next:
    
    ret = parse_fastlog_logdata(logdata, &log_id, &args_size, &rdtsc, &args_buff);

    printf("logId = %d, rdtsc = %ld, args size = %d\n", log_id, rdtsc, args_size);

    logdata = (((char*)logdata) + ret );

//    usleep(10000);

    goto parse_next;

    return ret;
}

static void release_and_exit()
{
    unmmap_fastlog_logfile(&ro_metadata_mmap);
    unmmap_fastlog_logfile(&ro_logdata_mmap);
    
    exit(0);
}

static void signal_handler(int signum)
{
    switch(signum) {
    case SIGINT:
        printf("Catch ctrl-C.\n");
        release_and_exit();
        break;
    }
    exit(1);
}

static int load_metadata_file(const char *mmapfile_name)
{
    int ret;
    
    //元数据文件映射
    ret = mmap_fastlog_logfile_read(&ro_metadata_mmap, mmapfile_name);

    metadata_header = ro_metadata_mmap.mmapaddr;

    if(metadata_header->magic != FATSLOG_METADATA_HEADER_MAGIC_NUMBER) {
        fprintf(stderr, "%s is not fastlog metadata file.\n", mmapfile_name);
        unmmap_fastlog_logfile(&ro_metadata_mmap);
        return -1;
    }
    

    return 0;
}

static int load_logdata_file(const char *mmapfile_name)
{
    int ret;

    //日志数据文件映射
    ret = mmap_fastlog_logfile_read(&ro_logdata_mmap, mmapfile_name);

    logdata_header  = ro_logdata_mmap.mmapaddr;

    if(logdata_header->magic != FATSLOG_LOG_HEADER_MAGIC_NUMBER) {
        fprintf(stderr, "%s is not fastlog logdata file.\n", mmapfile_name);
        unmmap_fastlog_logfile(&ro_logdata_mmap);
        return -1;
    }

    return 0;
}

static int match_metadata_and_logdata()
{
    uint64_t metadata_cycles_per_sec = metadata_header->cycles_per_sec;
    uint64_t metadata_start_rdtsc = metadata_header->start_rdtsc;
    uint64_t logdata_cycles_per_sec = logdata_header->cycles_per_sec;
    uint64_t logdata_start_rdtsc = logdata_header->start_rdtsc;

    /* 在写入时，这两个数值完全相等 */
    return ((metadata_cycles_per_sec == logdata_cycles_per_sec) 
        && (metadata_start_rdtsc == logdata_start_rdtsc));
}

/* 解析程序 主函数 */
int main(int argc, char *argv[])
{
    int ret;
    
    signal(SIGINT, signal_handler);

    __fastlog_cycles_init();

    /* 元数据文件的读取 */
    ret = load_metadata_file(FATSLOG_METADATA_FILE_DEFAULT);
    if(ret) {
        goto error;
    }

    /* 日志数据文件的读取 */
    ret = load_logdata_file(FATSLOG_LOG_FILE_DEFAULT);
    if(ret) {
        unmmap_fastlog_logfile(&ro_metadata_mmap); //释放元数据内存
        goto error;
    }

    /* 元数据和数据文件需要匹配，对比时间戳 */
    ret = match_metadata_and_logdata();
    if(!ret) {
        printf("metadata file and logdata file not match.\n");
        goto release;
    }

    /* 解析 元数据 */
    struct fastlog_metadata *metadata = metadata_header->data;
    parse_metadata(metadata);

    fastlog_logdata_t *logdata = logdata_header->data;
    parse_logdata(logdata);


release:
    release_and_exit();
    
error:
    return -1;
}

