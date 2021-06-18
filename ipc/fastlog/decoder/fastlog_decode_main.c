/**
 *  FastLog 低时延 LOG日志 解析工具
 *
 *  
 */
#include <fastlog_decode.h>
#include <fastlog_cycles.h>
#include <fastlog_decode_cli.h>
#include <fastlog_utils.h>

#include <getopt.h>


// fastlog decoder 配置参数，在 getopt 之后只读
struct fastlog_decoder_config decoder_config = {
    .decoder_version = "fq-decoder-1.0.0",
    .log_verbose_flag = true,  
    .boot_silence = false,
    .metadata_file = FATSLOG_METADATA_FILE_DEFAULT,
    .nr_log_files = 1,
    .logdata_file = FATSLOG_LOG_FILE_DEFAULT,
    .other_log_data_files = {NULL},
    .has_cli = true,

    /**
     *  默认日志显示级别
     *  
     *  数据类型为 `enum FASTLOG_LEVEL`
     *  FASTLOGLEVEL_ALL 表示全部，见命令行`command_helps`的`show level`
     *  当不开启命令行时，此变量决定要显示的日志级别
     */
    .default_log_level = FASTLOGLEVEL_ALL, 

    /**
     *  默认日志输出类型与输出文件
     *
     *  `output_type`输出类型为`LOG_OUTPUT_FILE_TYPE`
     *  `output_filename`输出文件名默认为空，标识默认为 console 输出(LOG_OUTPUT_FILE_CONSOLE)
     *          见命令行`command_helps`的`show level`
     */
    .output_type = LOG_OUTPUT_FILE_TXT,
    .output_filename_isset = false,
    .output_filename = DEFAULT_OUTPUT_FILE,
};
    
static void show_help(char *programname)
{
    printf("USAGE: %s\n", programname);
    printf("\n");
    printf(" [Info]\n");
    printf("  %3s %-15s \t: %s\n",   "-v,", "--version", "show version information");
    printf("  %3s %-15s \t: %s\n",   "-h,", "--help", "show this information");
    printf("\n");
    printf(" [Show]\n");
    printf("  %3s %-15s \t: %s\n",   "-V,", "--verbose", "show detail log information");
    printf("  %3s %-15s \t: %s\n",   "-B,", "--brief", "show brief log information");
    printf("  %3s %-15s \t: %s\n",   "-q,", "--quiet", "execute silence.");
    printf("\n");
    printf(" [Config]\n");
    printf("  %3s %-15s \t: %s\n",   "-M,", "--metadata [FILENAME]", "metadata file name");
    printf("  %3s %-15s \t: %s\n",   "-L,", "--logdata [FILENAME(s)]", "metadata file name");
    printf("  %3s %-15s \t  %s`%c`\n", " ",  "                      ", "FILENAMEs separate by ", LOGDATA_FILE_SEPERATOR);
    printf("  %3s %-15s \t  %s%c%s\n", " ",  "                      ", "Such as: -l file1.log",LOGDATA_FILE_SEPERATOR,"file2.log");
    printf("  %3s %-15s \t: %s\n",   "-c,", "--cli [OPTION]", "command line on or off. ");
    printf("  %3s %-15s \t  %s\n",   "   ", "              ", "OPTION=[on|off], default [on]");
    printf("  %3s %-15s \t: %s\n",   "-l,", "--log-level [OPTION]", "log level to output.");
    printf("  %3s %-15s \t  %s\n",   "   ", "                    ", "OPTION=[all|crit|err|warn|info|debug], default [all]");
    printf("  %3s %-15s \t: %s\n",   "-t,", "--log-type [OPTION]", "output type.");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", "OPTION=[txt|xml|json], default [txt]");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", "sometime may `xmlEscapeEntities : char out of range`,");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", " use `2>/dev/null` or -o log.xml");
    printf("  %3s %-15s \t: %s\n",   "-o,", "--output-file [FILENAME]", "output file name.");
    printf("  %3s %-15s \t  %s\n",   "   ", "                        ", "default "DEFAULT_OUTPUT_FILE);
    printf("\n");
    printf("FastLog Decoder(version %s).\n", decoder_config.decoder_version);
    printf("Developed by Rong Tao(2386499836@qq.com).\n");
}


static int parse_decoder_config(int argc, char *argv[])
{
    struct option options[] =
    {
        {"version", no_argument,    0,  'v'},
        {"help",    no_argument,    0,  'h'},
        {"verbose", no_argument,    0,  'V'},
        {"brief",   no_argument,    0,  'B'},
        {"quiet",   no_argument,    0,  'q'},
        {"metadata",    required_argument,     0,  'M'},
        {"logdata",     required_argument,     0,  'L'},
        {"cli",         required_argument,     0,  'c'},
        {"log-level",   required_argument,     0,  'l'},
        {"log-type",    required_argument,     0,  't'},
        {"output-file", required_argument,     0,  'o'},
        
        {0,0,0,0},
    };
        
    while (1) {
        int c;
        int option_index = 0;
        c = getopt_long (argc, argv, "vhVBqM:L:c:l:t:o:", options, &option_index);
        if(c < 0) {
            break;
        }
        switch (c) {
        case 0:
            /* If this option set a ﬂag, do nothing else now. */
            if (options[option_index].flag != 0)
                break;
            printf ("0 option %s", options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            
            printf ("\n");
            break;
            
        case 'v':
            printf("%s\n", decoder_config.decoder_version);
            exit(0);
            break;
            
        case 'h':
            show_help(argv[0]);
            exit(0);
            break;
            
        case 'V':
            decoder_config.log_verbose_flag = true;
            //printf("decoder_config.log_verbose_flag true.\n");
            break;
        case 'B':
            decoder_config.log_verbose_flag = false;
            //printf("decoder_config.log_verbose_flag false.\n");
            break;
        case 'q':
            decoder_config.boot_silence = true;
            //printf("decoder_config.log_verbose_flag false.\n");
            break;
            
        case 'M':
            decoder_config.metadata_file = strdup(optarg);
            assert(decoder_config.metadata_file);
            if(access(decoder_config.metadata_file, F_OK) != 0) {
                printf("Metadata file `%s` not exist.\n", decoder_config.metadata_file);
                free(decoder_config.metadata_file);
                printf("Check: %s -h, --help\n", argv[0]);
                exit(0);
            }
            //printf("Metadata file is `%s`\n", decoder_config.metadata_file);
            
            break;
            
        case 'L': {
            char* begin = optarg;

            /*  */
            while (1) {
                bool last_token = false;
                char* end = strchr(begin, LOGDATA_FILE_SEPERATOR);
                if (!end)
                {
                    last_token = true;
                }
                else
                {
                    *end = '\0';
                }

                if(strlen(begin) > 0) {
                    
                    //printf("begin = %s\n", begin);

                    /* 输入的文件必须存在 */
                    if(access(begin, F_OK) != 0) {
                        printf("Logdata file `%s` not exist.\n", begin);
                        printf("Check: %s -h, --help\n", argv[0]);
                        exit(0);
                    }
                    
                    if(decoder_config.nr_log_files == 1) {
                        decoder_config.logdata_file = strdup(begin);
                    } else {
                        decoder_config.other_log_data_files[decoder_config.nr_log_files-2] = strdup(begin);
                    }
                    
                    decoder_config.nr_log_files++;
                    if(decoder_config.nr_log_files > MAX_NUM_LOGDATA_FILES) {
                        printf("Logdata file is too much, limits %d\n", MAX_NUM_LOGDATA_FILES);
                        printf("Check: %s -h, --help\n", argv[0]);
                        exit(0);
                    }
                }

                if (last_token) {
                    break;
                } else {
                    begin = end + 1;
                }
            }
            decoder_config.nr_log_files --;
            //printf("Logdata file %d\n", decoder_config.nr_log_files);
            break;
        }
        
        case 'c':
            if(strcasecmp(optarg, "on") == 0) {
                decoder_config.has_cli = true;
            } else if(strcasecmp(optarg, "off") == 0) {
                decoder_config.has_cli = false;
            } else {
                printf("-c, --cli MUST has [on|off] argument.\n");
                printf("Check: %s -h, --help\n", argv[0]);
                exit(0);
            }

            break;
            
        case 'l':
            if(strncasecmp(optarg, "all", 3) == 0) {
                decoder_config.default_log_level = FASTLOGLEVEL_ALL;
            } else if(strncasecmp(optarg, "crit", 4) == 0) {
                decoder_config.default_log_level = FASTLOG_CRIT;
            } else if(strncasecmp(optarg, "err", 3) == 0) {
                decoder_config.default_log_level = FASTLOG_ERR;
            } else if(strncasecmp(optarg, "warn", 4) == 0) {
                decoder_config.default_log_level = FASTLOG_WARNING;
            } else if(strncasecmp(optarg, "info", 4) == 0) {
                decoder_config.default_log_level = FASTLOG_INFO;
            } else if(strncasecmp(optarg, "debug", 5) == 0) {
                decoder_config.default_log_level = FASTLOG_DEBUG;
            } else {
                printf("-l, --log-level MUST be one of all|crit|err|warn|info|debug.\n");
                printf("Check: %s -h, --help\n", argv[0]);
                exit(0);
            }
            break;
            
        case 't':
            if(strncasecmp(optarg, "txt", 3) == 0) {
                decoder_config.output_type = LOG_OUTPUT_FILE_TXT;
            } else if(strncasecmp(optarg, "xml", 3) == 0) {
                decoder_config.output_type = LOG_OUTPUT_FILE_XML;
            } else if(strncasecmp(optarg, "json", 4) == 0) {
                decoder_config.output_type = LOG_OUTPUT_FILE_JSON;
            } else {
                printf("-t, --log-type MUST be one of txt|xml|json.\n");
                printf("Check: %s -h, --help\n", argv[0]);
                exit(0);
            }
            break;

        
        case 'o':
            decoder_config.output_filename = strdup(optarg);
            assert(decoder_config.output_filename);
            if(access(decoder_config.output_filename, F_OK) == 0) {
                printf("Output file `%s` exist.\n", decoder_config.output_filename);
                free(decoder_config.output_filename);
                printf("Check: %s -h, --help\n", argv[0]);
                exit(0);
            }
            decoder_config.output_filename_isset = true;
            
            break;
            
        case '?':
            /* getopt_long already printed an error message. */
            printf ("option unknown ??????????\n");
            printf("Check: %s -h, --help\n", argv[0]);
            exit(0);
            break;
        default:
            abort ();
        }
    }
    
    //printf("decoder_config.log_verbose_flag = %d\n", decoder_config.log_verbose_flag);
    //printf("decoder_config.metadata_file    = %s\n", decoder_config.metadata_file);
    //printf("decoder_config.nr_log_files     = %d\n", decoder_config.nr_log_files);
    //printf("decoder_config.logdata_file     = %s\n", decoder_config.logdata_file);
    //printf("decoder_config.has_cli          = %d\n", decoder_config.has_cli);
    //printf("decoder_config.default_log_level= %d\n", decoder_config.default_log_level);
    //printf("decoder_config.output_type      = %d\n", decoder_config.output_type);
    //printf("decoder_config.output_filename  = %s\n", decoder_config.output_filename);
    /**
     *  解析参数后进行检查
     */
    //TODO
    
    //exit(0);
}



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
//    printf("_search logID %d\n", _search->log_id);

    metadata = (((char*)metadata) + ret );

    //轮询所有的元数据，直至到文件末尾
    if(metadata->magic == FATSLOG_METADATA_MAGIC_NUMBER) {
        goto parse_next;
    }

    return ret;
}


int parse_logdata(fastlog_logdata_t *logdata, size_t logdata_size)
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

    cli_exit();

    
    exit(0);
}

static void signal_handler(int signum)
{
    /* 开启命令行，请使用 quit exit命令退出 */
    //if(decoder_config.has_cli) {
    //    printf("input `quit` or `exit` to end.\n");
    //    return ;
    //}

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
    secondsSinceCheckpoint = __fastlog_cycles_to_seconds(tsc - meta_hdr()->start_rdtsc, 
                                        meta_hdr()->cycles_per_sec);
    
    int64_t wholeSeconds = (int64_t)(secondsSinceCheckpoint);
    nanos = 1.0e9 * (secondsSinceCheckpoint - (double)(wholeSeconds));
    time_t absTime = wholeSeconds + meta_hdr()->unix_time_sec;
    
    struct tm *_tm = localtime(&absTime);
    
    strftime(str_buffer, 32, "%Y-%m-%d/%T", _tm);
}


void metadata_print(struct metadata_decode *meta, void *arg)
{
    printf("metadata_print logID %d\n", meta->log_id);
}

void logdata_print_output(struct logdata_decode *logdata, void *arg)
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
    int load_logdata_count = 0;
    char *current_logdata_file = NULL;
    
    parse_decoder_config(argc, argv);
    
    signal(SIGINT, signal_handler);

    /* 初始化 cycles */
    __fastlog_cycles_init();

    /* 日志级别链表初始化 */
    level_lists__init();

    /* 元数据文件的读取 */
    ret = load_metadata_file(decoder_config.metadata_file);
    if(ret) {
        goto error;
    }

load_logdata:

    if(load_logdata_count == 0) {
        current_logdata_file = decoder_config.logdata_file;
    } else {
        current_logdata_file = decoder_config.other_log_data_files[load_logdata_count-1];
    }
    
    /* 日志数据文件的读取 */
    ret = load_logdata_file(current_logdata_file);
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

    
    /* 解析 元数据 */
    struct fastlog_metadata *metadata = meta_hdr()->data;
    parse_metadata(metadata);

    /**
     *  以下几行代码操作
     *
     *  1. 获取data日志信息 地址
     *  2. 读取日志(申请新的内存空间，将其加入 日志级别链表 和 日志红黑树)
     *  3. munmap 这个日志文件
     *
     *  注意：
     *  在调用 `release_logdata_file()` 后`log_mmapfile`和`log_hdr`不可用；
     *  需要使用`load_logdata_file`映射新的文件,并使用`match_metadata_and_logdata`检测和元数据匹配
     */
    fastlog_logdata_t *logdata = log_hdr()->data;
    parse_logdata(logdata, log_mmapfile()->mmap_size - sizeof(struct fastlog_file_header));
    release_logdata_file();

    /**
     *  这里使用 goto 语句遍历 `-L`参数输入的所有日志文件
     */
    load_logdata_count++;
    if(load_logdata_count < decoder_config.nr_log_files) {
        goto load_logdata;
    }
    
    struct output_struct *output = &output_txt;
    char *output_filename = NULL;

    /**
     *  输出类型
     */
    if(decoder_config.output_type & LOG_OUTPUT_FILE_TXT) {
        output = &output_txt;
    } else if(decoder_config.output_type & LOG_OUTPUT_FILE_XML) {
        output = &output_xml;
    } else if(decoder_config.output_type & LOG_OUTPUT_FILE_JSON) {
        output = &output_txt;
    } else {
        printf("just support txt, xml, json.\n");
        goto release;
    }

    /**
     *  是否设置了输出文件
     */
    if(decoder_config.output_filename_isset) {
        output_filename = decoder_config.output_filename;
    } 

    /* 如果以 quiet 模式启动，将不直接打印 */
    if(decoder_config.boot_silence) {
        goto quiet_boot;
    }
    
    output_open(output, output_filename);
    output_header(output, meta_hdr());


    if(decoder_config.default_log_level >= FASTLOG_CRIT && decoder_config.default_log_level < FASTLOGLEVELS_NUM) {
        level_list__iter(decoder_config.default_log_level, logdata_print_output, output);
    } else {
        logdata_rbtree__iter(logdata_print_output, output);
    }


#if 0 /* 几个遍历的 示例代码 */
    /* 遍历元数据 */
    metadata_rbtree__iter(metadata_print, output);

    /* 以日志级别遍历 */
    level_list__iter(FASTLOG_ERR, logdata_print_output, output);

    /* 以时间 tsc 寄存器的值 遍历 */
    logdata_rbtree__iter(logdata_print_output, output);

#endif
    
    output_footer(output);
    output_close(output);


quiet_boot:

    /**
     *  命令行默认是开启的
     */
    if(decoder_config.has_cli) {
        cli_init(); /* 命令行初始化 */
        cli_loop(); /* 命令行循环 */
        cli_exit(); /* 命令行释放 */
    }

    
release:
    release_and_exit();
    
error:
    return -1;
}

