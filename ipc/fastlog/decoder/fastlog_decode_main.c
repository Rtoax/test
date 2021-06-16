/**
 *  FastLog 低时延 LOG日志 解析工具
 *
 *  
 */
#include <fastlog_decode.h>
#include <fastlog_cycles.h>






int parse_fastlog_metadata(struct fastlog_metadata *metadata,
                int *log_id, int *level, char **name,
                char **file, char **func, int *line, char **format, 
                char **thread_name);

int parse_fastlog_logdata(fastlog_logdata_t *logdata, int *log_id, int *args_size, uint64_t *rdtsc, char **argsbuf);


static int parse_header(struct fastlog_file_header *header)
{
    /* 类型 */
    switch(header->magic) {
    case FATSLOG_METADATA_HEADER_MAGIC_NUMBER:
        printf("This is FastLog Metadata file.\n");
        break;
    case FATSLOG_LOG_HEADER_MAGIC_NUMBER:
        printf("This is FastLog LOG file.\n");
        break;
    default:
        printf("Wrong format of file.\n");
        return -1;
        break;
    }

    /* UTS */
	printf(
        "System Name:   %s\n"\
		"Release:       %s\n"\
		"Version:       %s\n"\
		"Machine:       %s\n"\
		"NodeName:      %s\n"\
		"Domain:        %s\n", 
        header->unix_uname.sysname, 
        header->unix_uname.release, 
        header->unix_uname.version, 
        header->unix_uname.machine,
        header->unix_uname.nodename, 
        /*no domainname*/ "no domainname");


    /* Record */
    {
    char buffer[256] = {0};
    struct tm _tm;
    localtime_r(&header->unix_time_sec, &_tm);
    strftime(buffer, 256, "Recoreded in:  %Y-%d-%m/%T", &_tm);
    printf("%s\n",buffer);
    }

    /*  */
    return 0;
}

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

    /* 创建保存 元数据和日志数据 的 红黑树 */
    metadata_rbtree__init();
    logdata_rbtree__init();

parse_next:

    if(metadata->magic != FATSLOG_METADATA_MAGIC_NUMBER) {
        assert(0 && "wrong format of metadata file.\n");
    }

    struct metadata_decode *decode_metadata = (struct metadata_decode*)malloc(sizeof(struct metadata_decode));
    assert(decode_metadata && "decode_metadata malloc  failed.");

    decode_metadata->log_id = metadata->log_id;
    decode_metadata->metadata = metadata;
    
    ret = parse_fastlog_metadata(metadata, &log_id, &level, &name, &file, &func, &line, &format, &thread_name);

    __fastlog_parse_format(format, &decode_metadata->argsType);

    decode_metadata->user_string = name;
    decode_metadata->src_filename = file;
    decode_metadata->src_function = func;
    decode_metadata->print_format = format;
    decode_metadata->thread_name = thread_name;


//    printf("log_id  = %d\n", log_id);
//    printf("thread  = %s\n", thread_name);
//    printf("level   = %d\n", level);
//    printf("line    = %d\n", line);
//    printf("name    = %s\n", name);
//    printf("file    = %s\n", file);
//    printf("func    = %s\n", func);
//    printf("format  = %s\n", format);
    
//    printf("insert logID %d\n", decode_metadata->log_id);

    metadata_rbtree__insert(decode_metadata);

//    struct metadata_decode *_search = metadata_rbtree__search(decode_metadata->log_id);
//
//    printf("_search logID %d\n", _search->log_id);


    metadata = (((char*)metadata) + ret );

    //轮询所有的元数据，直至到文件末尾
    if(metadata->magic == FATSLOG_METADATA_MAGIC_NUMBER) {
        goto parse_next;
    }


    return ret;
}


static int parse_logdata(fastlog_logdata_t *logdata, size_t logdata_size)
{
    int ret;
    
    int log_id; //所属ID
    int args_size;
    uint64_t rdtsc;
    char *args_buff;
    struct logdata_decode *log_decode;
    struct metadata_decode *metadata;

parse_next:
    
    ret = parse_fastlog_logdata(logdata, &log_id, &args_size, &rdtsc, &args_buff);

    if(log_id != 0 && logdata_size > 0) {

        metadata = metadata_rbtree__search(log_id);
        assert(metadata && "You gotta a wrong log_id");

        log_decode = (struct logdata_decode *)malloc(sizeof(struct logdata_decode));
        assert(log_decode && "Malloc <struct logdata_decode> failed." && __func__);
        memset(log_decode, 0, sizeof(struct logdata_decode));
        
        log_decode->logdata = (fastlog_logdata_t *)malloc(sizeof(fastlog_logdata_t) + args_size);
        assert(log_decode->logdata && "Malloc <fastlog_logdata_t> failed." && __func__);
        memset(log_decode->logdata, 0, sizeof(fastlog_logdata_t) + args_size);
        memcpy(log_decode->logdata, logdata, sizeof(fastlog_logdata_t) + args_size);

        
        log_decode->metadata = metadata;

        /* 将其插入链表中 */
        level_list__insert(metadata->metadata->log_level, log_decode);

        /* 插入到日志数据红黑树中 */
        logdata_rbtree__insert(log_decode);
        
        logdata = (((char*)logdata) + ret );
        logdata_size -= ret;

//        usleep(100000);

        goto parse_next;
    }
    
    return ret;
}


static void metadata_rbtree__rbtree_node_destroy(struct metadata_decode *node, void *arg)
{
//    printf("destroy logID %d\n", node->log_id);
    free(node);
}

static void logdata_rbtree__rbtree_node_destroy(struct logdata_decode *logdata, void *arg)
{
//    printf("destroy logID %d\n", node->log_id);
//    free(node);
    list_remove(&logdata->list_level[logdata->metadata->metadata->log_level]);
    free(logdata->logdata);
    free(logdata);
}

static void release_and_exit()
{
    metadata_rbtree__destroy(metadata_rbtree__rbtree_node_destroy, NULL);
    logdata_rbtree__destroy(logdata_rbtree__rbtree_node_destroy, NULL);
    
    release_metadata_file();
    release_logdata_file();

    
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

void timestamp_tsc_to_string(uint64_t tsc, char str_buffer[32])
{
    double secondsSinceCheckpoint, nanos = 0.0;
    secondsSinceCheckpoint = __fastlog_cycles_to_seconds(tsc - log_hdr()->start_rdtsc, 
                                        log_hdr()->cycles_per_sec);
    
    int64_t wholeSeconds = (int64_t)(secondsSinceCheckpoint);
    nanos = 1.0e9 * (secondsSinceCheckpoint - (double)(wholeSeconds));
    time_t absTime = wholeSeconds + log_hdr()->unix_time_sec;
    
    struct tm *_tm = localtime(&absTime);
    
    strftime(str_buffer, 32, "%Y-%m-%d/%T", _tm);
}


void metadata_print(struct metadata_decode *meta, void *arg)
{
    printf("metadata_print logID %d\n", meta->log_id);
}

void logdata_print(struct logdata_decode *logdata, void *arg)
{
    struct output_struct *output = (struct output_struct *)arg;

    // 从 "Hello, %s, %d" + World\02021 
    // 转化为
    // Hello, World, 2021
    reprintf(logdata, output);
}

/* 解析程序 主函数 */
int main(int argc, char *argv[])
{
    int ret;
    
    signal(SIGINT, signal_handler);

    /* 初始化 cycles */
    __fastlog_cycles_init();

    /* 日志级别链表初始化 */
    level_lists__init();

    /* 元数据文件的读取 */
    ret = load_metadata_file(FATSLOG_METADATA_FILE_DEFAULT);
    if(ret) {
        goto error;
    }

    /* 日志数据文件的读取 */
    ret = load_logdata_file(FATSLOG_LOG_FILE_DEFAULT);
    if(ret) {
        release_metadata_file(); //释放元数据内存
        goto error;
    }

    /* 元数据和数据文件需要匹配，对比时间戳 */
    ret = match_metadata_and_logdata();
    if(!ret) {
        printf("metadata file and logdata file not match.\n");
        goto release;
    }
    
    struct output_struct *output = &output_txt;

    output_open(output, NULL);
    
    /* 解析 元数据 */
    struct fastlog_metadata *metadata = meta_hdr()->data;
//    parse_header(meta_hdr());
    parse_metadata(metadata);
    
    fastlog_logdata_t *logdata = log_hdr()->data;
//    parse_header(log_hdr());
    parse_logdata(logdata, log_mmapfile()->mmap_size - sizeof(struct fastlog_file_header));


    output_header(output, log_hdr());

#if 1

    /* 遍历元数据 */
//    metadata_rbtree__iter(metadata_print, output);

    /* 以日志级别遍历 */
//    level_list__iter(FASTLOG_ERR, logdata_print, output);

    /* 以时间 tsc 寄存器的值 遍历 */
    logdata_rbtree__iter(logdata_print, output);

//    release_logdata_file();

#else

    printf("level count %ld/%ld\n", level_count(FASTLOG_ERR), level_count_all());
    printf(" meta count %ld\n", meta_count());
    printf("  log count %ld\n", log_count());
    
#endif
    
    output_footer(output);
    output_close(output);

release:
    release_and_exit();
    
error:
    return -1;
}

