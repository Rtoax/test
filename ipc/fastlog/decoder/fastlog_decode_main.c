/**
 *  FastLog 低时延 LOG日志 解析工具
 *
 *  
 */
#define _GNU_SOURCE
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
     *  `output_type`输出类型为`LOG_OUTPUT_TYPE`
     *  `output_filename`输出文件名默认为空，标识默认为 console 输出(LOG_OUTPUT_FILE_CONSOLE)
     *          见命令行`command_helps`的`show level`
     */
    .output_type = LOG_OUTPUT_FILE_TXT|LOG_OUTPUT_ITEM_HDR/*|LOG_OUTPUT_ITEM_LOG*/|LOG_OUTPUT_ITEM_FOOT,
    .output_filename_isset = false,
    .output_filename = DEFAULT_OUTPUT_FILE,

    
    .match_name = NULL,
    .match_func = NULL,
    .match_thread = NULL,
    .match_content = NULL,
};
    
static void show_help(char *programname)
{
    printf("USAGE: %s\n", programname);
    printf("\n");
    printf(" [Info]\n");
    printf("  %3s %-15s \t: %s\n",   "-v,", "--version", "show version information");
    printf("  %3s %-15s \t: %s\n",   "-h,", "--help", "show this information");
    printf("\n");
    printf(" [Show] Show option\n");
    printf("  %3s %-15s \t: %s\n",   "-V,", "--verbose", "show detail log information");
    printf("  %3s %-15s \t: %s\n",   "-B,", "--brief", "show brief log information");
    printf("  %3s %-15s \t: %s\n",   "-q,", "--quiet", "execute silence.");
    printf("  %3s %-15s \t: %s\n",   "-l,", "--log-level [OPTION]", "log level to output.");
    printf("  %3s %-15s \t  %s\n",   "   ", "                    ", "OPTION=[all|crit|err|warn|info|debug], default [all]");
    printf("  %3s %-15s \t: %s\n",   "-i,", "--log-item [OPTION(s)]", "log item to output.");
    printf("  %3s %-15s \t  %s\n",   "   ", "                    ", "OPTIONs=[meta|log], default [log]");
    printf("  %3s %-15s \t  %s`%c`\n", " ",  "                      ", "OPTIONs separate by ", LOGDATA_FILE_SEPERATOR);
    printf("  %3s %-15s \t  %s%c%s\n", " ",  "                      ", "Such as: -i meta",LOGDATA_FILE_SEPERATOR,"log");
    printf("  %3s %-15s \t  %s\n",   "   ",  "                      ", "     or: -i meta");
    printf("  %3s %-15s \t: %s\n",   "-t,", "--log-type [OPTION]", "output type.");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", "OPTION=[txt|xml|json], default [txt]");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", "sometime may `xmlEscapeEntities : char out of range`,");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", " use `2>/dev/null` or -o log.xml");
    printf("\n");
    printf(" [Filter] Filter option\n");
    printf("  %3s %-15s \t: %s\n",   "-N,", "--filter-name [NAME]", "FAST_LOG(level, name, ...)'s name");
    printf("  %3s %-15s \t: %s\n",   "-F,", "--filter-func [FUNC]", "function name");
    printf("  %3s %-15s \t: %s\n",   "-C,", "--filter-content [log]", "log content");
    printf("  %3s %-15s \t: %s\n",   "-T,", "--filter-thread [NAME]", "thread name");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", "May same as /proc/[PID]/comm");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", " or set by prctl(2) option PR_SET_NAME");
    printf("  %3s %-15s \t  %s\n",   "   ", "                   ", " or set by pthread_setname_np(3)");
    
    printf("\n");
    printf(" [Config] Configuration option\n");
    printf("  %3s %-15s \t: %s\n",   "-M,", "--metadata [FILENAME]", "metadata file name");
    printf("  %3s %-15s \t: %s\n",   "-L,", "--logdata [FILENAME(s)]", "metadata file name");
    printf("  %3s %-15s \t  %s`%c`\n", " ",  "                      ", "FILENAMEs separate by ", LOGDATA_FILE_SEPERATOR);
    printf("  %3s %-15s \t  %s%c%s\n", " ",  "                      ", "Such as: -l file1.log",LOGDATA_FILE_SEPERATOR,"file2.log");
    printf("  %3s %-15s \t: %s\n",   "-c,", "--cli [OPTION]", "command line on or off. ");
    printf("  %3s %-15s \t  %s\n",   "   ", "              ", "OPTION=[on|off], default [on]");
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
        {"log-item",    required_argument,     0,  'i'},
        {"log-type",    required_argument,     0,  't'},
        /**
         *  命令行过滤
         *  分别代表
         *  名称(模块名, 见`FAST_LOG(...)`接口的`name`入参)过滤
         *  函数名过滤
         *  日志内容过滤
         *  线程名过滤
         */
        {"filter-name",     required_argument,     0,  'N'},
        {"filter-func",     required_argument,     0,  'F'},
        {"filter-content",  required_argument,     0,  'C'},
        {"filter-thread",   required_argument,     0,  'T'},
        
        {"output-file", required_argument,     0,  'o'},
        
        {0,0,0,0},
    };
        
    while (1) {
        int c;
        int option_index = 0;
        c = getopt_long (argc, argv, "vhVBqM:L:c:l:i:t:N:F:C:T:o:", options, &option_index);
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
            
        case 'i':{
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
                    
                    if(strncasecmp(begin, "meta", 4) == 0) {
                        decoder_config.output_type |= LOG_OUTPUT_ITEM_META;
                    } else if(strncasecmp(begin, "log", 3) == 0) {
                        decoder_config.output_type |= LOG_OUTPUT_ITEM_LOG;
                    } else {
                        printf("-i, --log-item MUST be one of meta|log.\n");
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
            
            break;
        }
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

        case 'N':
            decoder_config.match_name = optarg;
            break;
        case 'F':
            decoder_config.match_func = optarg;
            break;
        case 'C':
            decoder_config.match_content = optarg;
            break;
        case 'T':
            decoder_config.match_thread = optarg;
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
            //printf ("option unknown ??????????\n");
            //printf("Check: %s -h, --help\n", argv[0]);
            show_help(argv[0]);
            exit(0);
            break;
        default:
            abort ();
        }
    }
    

    /**
     *  解析参数后进行检查
     */
    //如果 输出项未设置，使用默认值(日志，而不是元数据)
    if(!(decoder_config.output_type & LOG_OUTPUT_ITEM_META) 
    && !(decoder_config.output_type & LOG_OUTPUT_ITEM_LOG)) {
        decoder_config.output_type |= LOG_OUTPUT_ITEM_LOG;
    }
    //MORE
    
#if 0
    printf("decoder_config.log_verbose_flag = %d\n", decoder_config.log_verbose_flag);
    printf("decoder_config.metadata_file    = %s\n", decoder_config.metadata_file);
    printf("decoder_config.nr_log_files     = %d\n", decoder_config.nr_log_files);
    printf("decoder_config.logdata_file     = %s\n", decoder_config.logdata_file);
    printf("decoder_config.has_cli          = %d\n", decoder_config.has_cli);
    printf("decoder_config.default_log_level= %d\n", decoder_config.default_log_level);
    printf("decoder_config.output_type      = %#08x\n", decoder_config.output_type);
    printf("decoder_config.output_filename  = %s\n", decoder_config.output_filename);
    printf("decoder_config.match_name       = %s\n", decoder_config.match_name);
    printf("decoder_config.match_func       = %s\n", decoder_config.match_func);
    printf("decoder_config.match_content    = %s\n", decoder_config.match_content);
    printf("decoder_config.match_thread     = %s\n", decoder_config.match_thread);
    
    exit(0);
#endif

    return 0;
}



int parse_fastlog_metadata(struct fastlog_metadata *metadata,
                              int *log_id, int *level, char **name,
                              char **file, char **func, int *line, char **format, 
                              char **thread_name);

int parse_fastlog_logdata(fastlog_logdata_t *logdata, int *log_id, int *args_size, uint64_t *rdtsc, char **argsbuf);

static int _unused parse_header(struct fastlog_file_header *header)
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
    {
    char buffer[256] = {0};
    struct tm _tm;
    localtime_r(&header->unix_time_sec, &_tm);
    strftime(buffer, 256, "Recoreded in:  %Y-%d-%m/%T", &_tm);
    printf("%s\n",buffer);
    }
#pragma GCC diagnostic pop

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
    log_search_rbtree__init();
    
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
    id_lists__init_raw(decode_metadata);
    
//    struct metadata_decode *_search = metadata_rbtree__search(decode_metadata->log_id);
//    printf("_search logID %d\n", _search->log_id);

    metadata = (struct fastlog_metadata *)(((char*)metadata) + ret );

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
    int _unused i;
    
    uint64_t rdtsc;
    char *args_buff;
    struct logdata_decode *log_decode;
    struct metadata_decode *metadata;

parse_next:
    
    ret = parse_fastlog_logdata(logdata, &log_id, &args_size, &rdtsc, &args_buff);

    if(log_id != 0 && logdata_size > 0) {

        metadata = metadata_rbtree__search(log_id);
        //printf("LOGID %d's metadata is exist.\n", log_id);
        if(unlikely(!metadata)) {
            printf("LOGID %d's metadata is not exist.\n", log_id);
            printf("args_size = %d\n", args_size);
            assert(metadata && "You gotta a wrong log_id");
            //goto parse_next;
        }

        log_decode = (struct logdata_decode *)malloc(sizeof(struct logdata_decode));
        assert(log_decode && "Malloc <struct logdata_decode> failed.");
        memset(log_decode, 0, sizeof(struct logdata_decode));
        
        log_decode->logdata = (fastlog_logdata_t *)malloc(sizeof(fastlog_logdata_t) + args_size);
        assert(log_decode->logdata && "Malloc <fastlog_logdata_t> failed.");
        memset(log_decode->logdata, 0, sizeof(fastlog_logdata_t) + args_size);
        memcpy(log_decode->logdata, logdata, sizeof(fastlog_logdata_t) + args_size);

        
        log_decode->metadata = metadata;

        /* 将其插入链表中 */
        level_list__insert(metadata->metadata->log_level, log_decode);

        /* 插入到 log_id 链表中 */
        id_list__insert_raw(metadata, log_decode);

        /* 将 log_id 标记入 bitmask */
        log_ids__set(log_id);

        /* 插入到日志数据红黑树中 */
        logdata_rbtree__insert(log_decode);

        struct log_search _unused *search_func = NULL;

        search_func = log_search_rbtree__search_or_create(LOG__RANGE_FUNC_ENUM, log_decode->metadata->src_function);
        log_search_list__insert(search_func, log_decode);
        search_func = log_search_rbtree__search_or_create(LOG__RANGE_NAME_ENUM, log_decode->metadata->user_string);
        log_search_list__insert(search_func, log_decode);
        search_func = log_search_rbtree__search_or_create(LOG__RANGE_THREAD_ENUM, log_decode->metadata->thread_name);
        log_search_list__insert(search_func, log_decode);
        
        logdata = (fastlog_logdata_t *)(((char*)logdata) + ret );
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
    list_remove(&logdata->list_level);
    free(logdata->logdata);
    free(logdata);
}

static void log_search_rbtree__rbtree_node_destroy(struct log_search *search, void *arg)
{
//    printf("destroy search %d\n", search->string_type);
    free(search);
}

static void release_and_exit()
{
    metadata_rbtree__destroy(metadata_rbtree__rbtree_node_destroy, NULL);
    logdata_rbtree__destroy(logdata_rbtree__rbtree_node_destroy, NULL);

    log_search_rbtree__destroyall(log_search_rbtree__rbtree_node_destroy, NULL);
    
    release_metadata_file();
    release_logdata_file();

    cli_exit();

    log_ids__destroy();
    
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
    double secondsSinceCheckpoint, _unused nanos = 0.0;
    secondsSinceCheckpoint = __fastlog_cycles_to_seconds(tsc - meta_hdr()->start_rdtsc, 
                                        meta_hdr()->cycles_per_sec);
    
    int64_t wholeSeconds = (int64_t)(secondsSinceCheckpoint);
    //nanos = 1.0e9 * (secondsSinceCheckpoint - (double)(wholeSeconds));
    time_t absTime = wholeSeconds + meta_hdr()->unix_time_sec;
    
    struct tm *_tm = localtime(&absTime);
    
    strftime(str_buffer, 32, "%Y-%m-%d/%T", _tm);
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

    /* 初始化 log_id bitmask */
    log_ids__init();

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
    struct fastlog_metadata *metadata = (struct fastlog_metadata *)meta_hdr()->data;
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
    fastlog_logdata_t *logdata = (fastlog_logdata_t *)log_hdr()->data;
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
        output = &output_json;
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

#if 0 /* 测试 */
    
    log_search_list__iter2(LOG__RANGE_FUNC_ENUM, "main", output_logdata, output);
    log_search_list__iter2(LOG__RANGE_FUNC_ENUM, NULL, output_logdata, output);

    log_search_list__iter3(LOG__RANGE_FUNC_ENUM, "test3", output_logdata, output);

    exit(0);

#endif

    /*
     *  过滤器 
     *
     *  根据入参决定 filter 规则，当前支持四种(见`__LOG__RANGE_FILTER_NUM`)
     */
    if(decoder_config.match_name) {
        struct output_filter_arg arg1 = {
            .name = decoder_config.match_name,
        };
        output_setfilter(output, &filter_name, arg1);
    }

    if(decoder_config.match_func) {
        struct output_filter_arg arg2 = {
            .func = decoder_config.match_func,
        };
        output_setfilter(output, &filter_func, arg2);
    }
    if(decoder_config.match_content) {
        struct output_filter_arg arg3 = {
            .content = decoder_config.match_content,
        };
        output_setfilter(output, &filter_content, arg3);
    }
    if(decoder_config.match_thread) {
        struct output_filter_arg arg4 = {
            .thread = decoder_config.match_thread,
        };
        output_setfilter(output, &filter_thread, arg4);
    }


    
    //是否输出元数据
    if(decoder_config.output_type & LOG_OUTPUT_ITEM_META) {
        metadata_rbtree__iter_level(decoder_config.default_log_level, output_metadata, output);
    }
    
    //是否输出日志数据
    if(decoder_config.output_type & LOG_OUTPUT_ITEM_LOG) {
        if(decoder_config.default_log_level >= FASTLOG_CRIT && decoder_config.default_log_level < FASTLOGLEVELS_NUM) {
            level_list__iter(decoder_config.default_log_level, output_logdata, output);
        } else {
            logdata_rbtree__iter(output_logdata, output);
        }
    }

#if 0 /* 几个遍历的 示例代码 */
    
    /* 遍历元数据 */
    metadata_rbtree__iter(output_metadata, output);

    /* 以日志级别遍历 */
    level_list__iter(FASTLOG_ERR, output_logdata, output);

    /* 以时间 tsc 寄存器的值 遍历 */
    logdata_rbtree__iter(output_logdata, output);

    printf("show log ID.\n");
    ret = id_list__iter(3, output_logdata, output);
    if(ret) {
        printf("logid %d not exit.\n", 3);
    }
#endif
    
    output_footer(output);
    output_close(output);




quiet_boot:

//    void log_callback(struct logdata_decode *logdata, void *arg) {
//        printf("log_id = %d\n", logdata->logdata->log_id);
//
//    }
//    void id_callback(int log_id, void *arg) {
//        id_list__iter(log_id, log_callback, NULL);
//    }
//
//    log_ids__iter(id_callback, NULL);

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

