#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>

#include <fastlog_decode.h>
#include <fastlog_decode_cli.h>

#include <linenoise/linenoise.h>
#include <hiredis/sds.h>


#define CLI_HELP_COMMAND    1
#define CLI_HELP_GROUP      2


struct command_help {
    char *name;
    char *params;
    char *summary;
    int group;
};

typedef struct {
    int type;
    int argc;
    sds *argv;
    sds full;

    /* Only used for help on commands */
    struct command_help *org;
} help_entry;


static help_entry *help_entries;
static int help_entries_len;

#define GRP_SHOW    0
#define GRP_SAVE    1
#define GRP_LOAD    2
#define GRP_QUIT    3

static char *command_groups[] = {
    "show",
    "save",
    "load",
    "quit",
};

enum {
    CMD_SHOW_HELP,
    CMD_SHOW_CMD_LIST,
    CMD_SHOW_LOG,
    CMD_SHOW_LOGID,
    CMD_SHOW_META,
    CMD_SHOW_LS,
    CMD_SHOW_SEARCH,
    CMD_LOAD_LOG,
    CMD_QUIT_QUIT,
    CMD_QUIT_EXIT,
    CMD_MAX_NUM,
};
struct command_help command_helps[] = {
    
    [CMD_SHOW_HELP] = { 
        .name = "show",
        .params = "help",
        .summary = "show help",
        .group = GRP_SHOW,
    },
    [CMD_SHOW_CMD_LIST] = { 
        .name = "list",
        .params = "",
        .summary = "show all command list",
        .group = GRP_SHOW,
    },
    [CMD_SHOW_LOG] = { 
        .name = "log",
        .params = "all|crit|err|warn|info|debug txt|xml|json [FILENAME]",
        .summary = "show log by level and save to txt,xml,json FILENAME",
        .group = GRP_SHOW,
    },
    [CMD_SHOW_LOGID] = { 
        .name = "logid",
        .params = "id all|crit|err|warn|info|debug txt|xml|json [FILENAME]",
        .summary = "show log by level and save to txt,xml,json FILENAME",
        .group = GRP_SHOW,
    },
    [CMD_SHOW_META] = {
        .name = "meta",
        .params = "all|crit|err|warn|info|debug txt|xml|json [FILENAME]",
        .summary = "show metadata and save to txt,xml,json FILENAME",
        .group = GRP_SHOW,
    },
    [CMD_SHOW_LS] = { 
        .name = "ls",
        .params = "",
        .summary = "system ls command",
        .group = GRP_SHOW,
    },
    [CMD_SHOW_SEARCH] = { 
        .name = "search",
        .params = "func|name|content|thread value all|crit|err|warn|info|debug txt|xml|json [FILENAME]",
        .summary = "search information",
        .group = GRP_SHOW,
    },
    [CMD_LOAD_LOG] = { 
        .name = "load",
        .params = "log [FILENAME]",
        .summary = "load a new logdata file, named by FILENAME",
        .group = GRP_LOAD,
    },
    [CMD_QUIT_QUIT] = { 
        .name = "quit",
        .params = "",
        .summary = "quit program",
        .group = GRP_QUIT,
    },
    [CMD_QUIT_EXIT] = { 
        .name = "exit",
        .params = "",
        .summary = "quit program",
        .group = GRP_QUIT,
    },
};

char cli_prompt[64]  ={"FastLog>> "};


static void show_command_list()
{
    int i;
    
    printf("\n");
    printf("\t input CMD help to check command manual.\n");
    printf("\n");
    
    for (i=0; i<CMD_MAX_NUM; i++) {
        printf("\t %10s: %s\n", 
                            command_helps[i].name, 
                            command_helps[i].summary);
    }
}

static void show_help()
{
    printf("\n");
    printf("\t `show help`: show this information.\n");
    printf("\n");
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_HELP].name, 
                        command_helps[CMD_SHOW_HELP].params, 
                        command_helps[CMD_SHOW_HELP].summary);
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_CMD_LIST].name, 
                        command_helps[CMD_SHOW_CMD_LIST].params, 
                        command_helps[CMD_SHOW_CMD_LIST].summary);
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_LOG].name, 
                        command_helps[CMD_SHOW_LOG].params, 
                        command_helps[CMD_SHOW_LOG].summary);
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_LOGID].name, 
                        command_helps[CMD_SHOW_LOGID].params, 
                        command_helps[CMD_SHOW_LOGID].summary);
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_META].name, 
                        command_helps[CMD_SHOW_META].params, 
                        command_helps[CMD_SHOW_META].summary);
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_LS].name, 
                        command_helps[CMD_SHOW_LS].params, 
                        command_helps[CMD_SHOW_LS].summary);
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_SHOW_SEARCH].name, 
                        command_helps[CMD_SHOW_SEARCH].params, 
                        command_helps[CMD_SHOW_SEARCH].summary);
    printf("\n");
}

static void load_help()
{
    printf("\n");
    printf("\t `load help`: show this information.\n");
    printf("\n");
    printf("\t %10s %s: %s\n", 
                        command_helps[CMD_LOAD_LOG].name, 
                        command_helps[CMD_LOAD_LOG].params, 
                        command_helps[CMD_LOAD_LOG].summary);
    
    printf("\n");
}

static void cliInitHelp(void)
{
    int commandslen = sizeof(command_helps)/sizeof(struct command_help);
    int groupslen = sizeof(command_groups)/sizeof(char*);
    int i, len, pos = 0;
    help_entry tmp;

    help_entries_len = len = commandslen+groupslen;
    help_entries = malloc(sizeof(help_entry)*len);

    for (i = 0; i < groupslen; i++) {
        tmp.argc = 1;
        tmp.argv = malloc(sizeof(sds));
        tmp.argv[0] = sdscatprintf(sdsempty(),"@%s",command_groups[i]);
        tmp.full = tmp.argv[0];
        tmp.type = CLI_HELP_GROUP;
        tmp.org = NULL;
        help_entries[pos++] = tmp;
    }

    for (i = 0; i < commandslen; i++) {
        tmp.argv = sdssplitargs(command_helps[i].name,&tmp.argc);
        tmp.full = sdsnew(command_helps[i].name);
        tmp.type = CLI_HELP_COMMAND;
        tmp.org = &command_helps[i];
        help_entries[pos++] = tmp;
    }
}

/* Linenoise completion callback. */
static void decoder_completionCallback(const char *buf, linenoiseCompletions *lc) 
{
    size_t startpos = 0;
    int mask;
    int i;
    size_t matchlen;
    sds tmp;

    if (strncasecmp(buf,"help ",5) == 0) {
        startpos = 5;
        while (isspace(buf[startpos])) startpos++;
        mask = CLI_HELP_COMMAND | CLI_HELP_GROUP;
    } else {
        mask = CLI_HELP_COMMAND;
    }

    for (i = 0; i < help_entries_len; i++) {
        if (!(help_entries[i].type & mask)) continue;

        matchlen = strlen(buf+startpos);
        if (strncasecmp(buf+startpos, help_entries[i].full, matchlen) == 0) {
            tmp = sdsnewlen(buf,startpos);
            tmp = sdscat(tmp, help_entries[i].full);
            linenoiseAddCompletion(lc,tmp);
            sdsfree(tmp);
        }
    }
}


/* Linenoise hints callback. */
static char *decoder_hintsCallback(const char *buf, int *color, int *bold)
{
    int i, argc, buflen = strlen(buf);
    sds *argv = sdssplitargs(buf,&argc);
    int endspace = buflen && isspace(buf[buflen-1]);

    /* Check if the argument list is empty and return ASAP. */
    if (argc == 0) {
        sdsfreesplitres(argv,argc);
        return NULL;
    }

    for (i = 0; i < help_entries_len; i++) {
        if (!(help_entries[i].type & CLI_HELP_COMMAND)) continue;

        if (strcasecmp(argv[0],help_entries[i].full) == 0 ||
            strcasecmp(buf,help_entries[i].full) == 0)
        {
            *color = 90;
            *bold = 0;
            sds hint = sdsnew(help_entries[i].org->params);

            /* Remove arguments from the returned hint to show only the
             * ones the user did not yet typed. */
            int toremove = argc-1;
            while(toremove > 0 && sdslen(hint)) {
                if (hint[0] == '[') break;
                if (hint[0] == ' ') toremove--;
                sdsrange(hint,1,-1);
            }

            /* Add an initial space if needed. */
            if (!endspace) {
                sds newhint = sdsnewlen(" ",1);
                newhint = sdscatsds(newhint,hint);
                sdsfree(hint);
                hint = newhint;
            }

            sdsfreesplitres(argv,argc);
            return hint;
        }
    }
    sdsfreesplitres(argv,argc);
    return NULL;
}

static void decoder_freeHintsCallback(void *ptr)
{
    sdsfree(ptr);
}


static sds *cliSplitArgs(char *line, int *argc)
{
        return sdssplitargs(line,argc);
}


void cli_init()
{

    linenoiseSetMultiLine(1);

    cliInitHelp();


    linenoiseSetCompletionCallback(decoder_completionCallback);
    linenoiseSetHintsCallback(decoder_hintsCallback);

    linenoiseSetFreeHintsCallback(decoder_freeHintsCallback);
    
    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("fastlog.cli.history.txt"); /* Load the history at startup */

}

static int cmd_show_level_getarg(int argc, int argc_off, char **argv, 
                    enum FASTLOG_LEVEL *log_level, LOG_OUTPUT_TYPE *file_type, char **filename)
{
    if(argc == 1+argc_off) {
        return -2;
    }
    
    /* 日志级别 */
    if(strncasecmp(argv[1+argc_off], "all", 3) == 0) {
        *log_level = FASTLOGLEVEL_ALL;
    } else if(strncasecmp(argv[1+argc_off], "crit", 4) == 0) {
        *log_level = FASTLOG_CRIT;
    } else if(strncasecmp(argv[1+argc_off], "err", 3) == 0) {
        *log_level = FASTLOG_ERR;
    } else if(strncasecmp(argv[1+argc_off], "warn", 4) == 0) {
        *log_level = FASTLOG_WARNING;
    } else if(strncasecmp(argv[1+argc_off], "info", 4) == 0) {
        *log_level = FASTLOG_INFO;
    } else if(strncasecmp(argv[1+argc_off], "debug", 5) == 0) {
        *log_level = FASTLOG_DEBUG;
    } else {
        printf("\t show level MUST be one of all|crit|err|warn|info|debug.\n");
        return -1;
    }

    /* 文件格式 */
    if(argc >= 3+argc_off) {
        if(strncasecmp(argv[2+argc_off], "txt", 3) == 0) {
            *file_type = LOG_OUTPUT_FILE_TXT;
        } else if(strncasecmp(argv[2+argc_off], "xml", 3) == 0) {
            *file_type = LOG_OUTPUT_FILE_XML;
        } else if(strncasecmp(argv[2+argc_off], "json", 4) == 0) {
            *file_type = LOG_OUTPUT_FILE_JSON;
        } else {
            printf("\t show file type MUST be one of txt|xml|json.\n");
            return -1;
        }
    }

    /* 文件名 */
    if(argc >= 4+argc_off) {
        *filename = argv[3+argc_off];
        if(access(*filename, F_OK) == 0) {
            printf("\t `%s` already exist.\n", *filename);
            return -1;
        }
    } else {
        // 未设置文件名，默认向 console 输出
        *file_type |= LOG_OUTPUT_FILE_CONSOLE;
    }

    return 0;
}

/**
 *  show 显示 API
 *
 *  过滤条件下可用如下方式调用:
 *  `cmd_show_level(log_level, file_type, filename, "TEST2", "routine", "rtoax:2", "World");`
 *
 *  
 */
static void cmd_show_level(enum FASTLOG_LEVEL log_level, LOG_OUTPUT_TYPE file_type, char * filename,
                           char *match_name, char *match_func, char *match_thread, char *match_content,
                           int log_id)
{
    /* 是否输出文件 */
    char *open_filename = NULL;
    if(filename) {
        open_filename = filename;
    }
    
    struct output_struct *output = NULL;
    
    if(file_type & LOG_OUTPUT_FILE_TXT) {
        output = &output_txt;
    } else if(file_type & LOG_OUTPUT_FILE_XML) {
        output = &output_xml;
    } else if(file_type & LOG_OUTPUT_FILE_JSON) {
        output = &output_json;
    } else {
        printf("\t just support txt, xml, json.\n");
        return;
    }
    
    output_open(output, open_filename);

    //是否输出头部
    if(file_type & LOG_OUTPUT_ITEM_HDR) {
        output_header(output, meta_hdr());
    }

    /*
     *  过滤器 
     *
     *  根据入参决定 filter 规则，当前支持四种(见`__LOG__RANGE_FILTER_NUM`)
     */
    if(match_name) {
        struct output_filter_arg arg1 = {
            .name = match_name,
        };
        output_setfilter(output, &filter_name, arg1);
    }

    if(match_func) {
        struct output_filter_arg arg2 = {
            .func = match_func,
        };
        output_setfilter(output, &filter_func, arg2);
    }
    if(match_content) {
        struct output_filter_arg arg3 = {
            .content = match_content,
        };
        output_setfilter(output, &filter_content, arg3);
    }
    if(match_thread) {
        struct output_filter_arg arg4 = {
            .thread = match_thread,
        };
        output_setfilter(output, &filter_thread, arg4);
    }

    //是否输出元数据
    if(file_type & LOG_OUTPUT_ITEM_META) {
        metadata_rbtree__iter_level(log_level, output_metadata, output);
    }
    
    //是否输出日志数据
    if(file_type & LOG_OUTPUT_ITEM_LOG) {

        //如果输入了特定的 log_id
        if(log_id == 0) {
            if(log_level >= FASTLOG_CRIT && log_level < FASTLOGLEVELS_NUM) {
                level_list__iter(log_level, output_logdata, output);
            } else {
                logdata_rbtree__iter(output_logdata, output);
            }

        //指定输出特定的 LOG ID
        } else {
            id_list__iter(log_id, output_logdata, output);
        }
    }
    
    //是否输出尾部
    if(file_type & LOG_OUTPUT_ITEM_FOOT) {
        output_footer(output);
    }
    
    output_close(output);

    printf("\n");
    //printf("show level %s 0x%04x %s.\n", strlevel_color(log_level), file_type, filename?filename:"null");
}

static void cmd_load_log(char *filename)
{
    int ret;
    
    /* 日志数据文件的读取 */
    ret = load_logdata_file(filename);
    if(ret) {
        printf("\t load_logdata_file %s failed.\n", filename);
        goto error;
    }
    /* 元数据和数据文件需要匹配，对比时间戳 */
    ret = match_metadata_and_logdata();
    if(!ret) {
        printf("\t metadata file and logdata file not match.\n");
        goto error;
    }
    
    fastlog_logdata_t *logdata = (fastlog_logdata_t *)log_hdr()->data;
    parse_logdata(logdata, log_mmapfile()->mmap_size - sizeof(struct fastlog_file_header));

    printf("\t Load fastlog logdata file `%s` success.\n", filename);
    
error:
    release_logdata_file(); //释放元数据内存
    return;
}


static int invoke_command(int argc, char **argv, long repeat)
{
    int ret;
    
    //show 
    if(strncasecmp(argv[0], "show", 4) == 0) {
        if(argc == 1) {
            printf("\t input `show help` to check show command.\n");
            return 0;
        } else if(argc >= 2) {
            //show help
            if(strncasecmp(argv[1], "help", 4) == 0) {
                show_help();
                return 0;
            }
        }
    } 
    //log 
    else if(strcasecmp(argv[0], "log") == 0) {

        /*all|crit|err|warn|info|debug txt|xml|json [FILENAME] */
        enum FASTLOG_LEVEL log_level = FASTLOG_ERR;
        LOG_OUTPUT_TYPE file_type = LOG_OUTPUT_FILE_TXT;
        char *filename = NULL;
        
        ret = cmd_show_level_getarg(argc, 0, argv, &log_level, &file_type, &filename);
        if(ret) {
            printf("\t input `show help` to check show command.\n");
            return 0;
        }

        // 默认输出所有项
        file_type |= LOG_OUTPUT_ITEM_LOG_MASK;

        cmd_show_level(log_level, file_type, filename, NULL, NULL, NULL, NULL, 0);

    }
    //logid
    else if(strcasecmp(argv[0], "logid") == 0) {
        
        /*id all|crit|err|warn|info|debug txt|xml|json [FILENAME] */
        int log_id = 0;
        
        enum FASTLOG_LEVEL log_level = FASTLOGLEVEL_ALL;
        LOG_OUTPUT_TYPE file_type = LOG_OUTPUT_FILE_TXT;
        char *filename = NULL;

        if(argc >= 2) {
            log_id = atoi(argv[1]);
            if(log_id == 0) {
                printf("\t wrong log_id, input `meta` cmd check log_id.\n");
                return 0;
            }
        }
        if(argc >= 3) {
//            ret = cmd_show_level_getarg(argc, 1, argv, &log_level, &file_type, &filename);
//            if(ret == -2) {
//                printf("\t input `show help` to check show command.\n");
//                return 0;
//            }
        }

        // 默认输出所有项
        file_type |= LOG_OUTPUT_ITEM_LOG_MASK;

        cmd_show_level(log_level, file_type, filename, NULL, NULL, NULL, NULL, log_id);
    }
    //meta
    else if(strncasecmp(argv[0], "meta", 4) == 0) {

        /*all|crit|err|warn|info|debug txt|xml|json [FILENAME] */
        enum FASTLOG_LEVEL log_level = FASTLOG_ERR;
        LOG_OUTPUT_TYPE file_type = LOG_OUTPUT_FILE_TXT;
        char *filename = NULL;
        
        ret = cmd_show_level_getarg(argc, 0, argv, &log_level, &file_type, &filename);
        if(ret) {
            printf("\t input `show help` to check show command.\n");
            return 0;
        }
        
        // 默认输出所有项
        file_type |= LOG_OUTPUT_ITEM_META_MASK;
        
        cmd_show_level(log_level, file_type, filename, NULL, NULL, NULL, NULL, 0);
    
    } 
    //ls
    else if(strncasecmp(argv[0], "ls", 2) == 0) {
        system("ls ./");

    } 
    //load
    else if(strncasecmp(argv[0], "load", 4) == 0) {
        if(argc == 1) {
            printf("\t input `load help` to check show command.\n");
            return 0;
        } else if(argc >= 2) {
            //load help
            if(strncasecmp(argv[1], "help", 4) == 0) {
                load_help();
                return 0;
            //load log
            } else if(strncasecmp(argv[1], "log", 3) == 0) {
                char *filename = NULL;
                /* 文件名 */
                if(argc >= 3) {
                    filename = argv[2];
                    if(access(filename, F_OK) != 0) {
                        printf("\t Input file `%s` not exist.\n", filename);
                        return 0;
                    }

                    cmd_load_log(filename);
                } else {
                    printf("\t MUST Input file name.\n");
                }
                
                return 0;
            } else {
                printf("\t input `load help` to check load command.\n");
                return 0;
            }
        }
    } 
    //list
    else if(strncasecmp(argv[0], "list", 4) == 0)  {
        show_command_list();
        
    } 
    //search
    else if(strncasecmp(argv[0], "search", 6) == 0) {

        //func|name|content|thread value
    
        char *search_type = NULL;
        char *search_value = NULL;
        char *search_values[__LOG__RANGE_FILTER_NUM] = {NULL};

        LOG_OUTPUT_TYPE file_type = LOG_OUTPUT_FILE_TXT;
        enum FASTLOG_LEVEL log_level = FASTLOGLEVEL_ALL;
        char *filename = NULL;
        
        int argv_idx = 0;
        
        if(argc <= 2) {
            printf("\t Must has one [func|name|content|thread value] pair. Like `search func main`\n");
            return 0;
            
        } else if(argc >= 3) {
            
            argv_idx = 1;
            
            search_type = argv[argv_idx];
            search_value = argv[argv_idx+1];
            
            if(strncasecmp(search_type, "func", 4) == 0) {
                search_values[LOG__RANGE_FUNC_ENUM] = search_value;
            } else if(strncasecmp(search_type, "name", 4) == 0) {
                search_values[LOG__RANGE_NAME_ENUM] = search_value;
            } else if(strncasecmp(search_type, "content", 7) == 0) {
                search_values[LOG__RANGE_CONTENT_ENUM] = search_value;
            } else if(strncasecmp(search_type, "thread", 6) == 0) {
                search_values[LOG__RANGE_THREAD_ENUM] = search_value;
            } else { 
                printf("\t search item MUST be one of `func|name|content|thread`.\n");
                return -1;
            }
            
            argv_idx += 2;      /* 参数成对出现 */

            if(argc > argv_idx) {
    
                /* 日志级别 */
                if(strncasecmp(argv[argv_idx], "all", 3) == 0) {
                    log_level = FASTLOGLEVEL_ALL;
                } else if(strncasecmp(argv[argv_idx], "crit", 4) == 0) {
                    log_level = FASTLOG_CRIT;
                } else if(strncasecmp(argv[argv_idx], "err", 3) == 0) {
                    log_level = FASTLOG_ERR;
                } else if(strncasecmp(argv[argv_idx], "warn", 4) == 0) {
                    log_level = FASTLOG_WARNING;
                } else if(strncasecmp(argv[argv_idx], "info", 4) == 0) {
                    log_level = FASTLOG_INFO;
                } else if(strncasecmp(argv[argv_idx], "debug", 5) == 0) {
                    log_level = FASTLOG_DEBUG;
                } else {
                    printf("\t show level MUST be one of all|crit|err|warn|info|debug.\n");
                    return -1;
                }
                
                argv_idx++;

                if(argc > argv_idx) {
                    /* 文件格式 */
                    if(strncasecmp(argv[argv_idx], "txt", 3) == 0) {
                        file_type = LOG_OUTPUT_FILE_TXT;
                    } else if(strncasecmp(argv[argv_idx], "xml", 3) == 0) {
                        file_type = LOG_OUTPUT_FILE_XML;
                    } else if(strncasecmp(argv[argv_idx], "json", 4) == 0) {
                        file_type = LOG_OUTPUT_FILE_JSON;
                    } else {
                        printf("\t show file type MUST be one of txt|xml|json.\n");
                        return -1;
                    }
                    argv_idx++;
                }
                
                if(argc > argv_idx) {
                    filename = argv[argv_idx];
                    if(access(filename, F_OK) == 0) {
                        printf("\t `%s` already exist.\n", filename);
                        return -1;
                    }
                }

                
            }


//            printf("\t log_level = %x\n", log_level);
//            printf("\t file type = %x\n", file_type);
//            printf("\t filename  = %s\n", filename);
//            
//            for(i=0; i<__LOG__RANGE_FILTER_NUM; i++) {
//                if(search_values[i])
//                    printf("\t search %d value %s\n", i, search_values[i]);
//            }
            
            file_type |= LOG_OUTPUT_ITEM_LOG_MASK;
            
            cmd_show_level(log_level, file_type, filename, 
                            search_values[LOG__RANGE_NAME_ENUM], 
                            search_values[LOG__RANGE_FUNC_ENUM], 
                            search_values[LOG__RANGE_THREAD_ENUM], 
                            search_values[LOG__RANGE_CONTENT_ENUM], 0);
        }

    
    } else {
        printf("\t input `list` to check all command.\n");
    }
    return 0;
}


void cli_loop()
{
    char *line;
    sds historyfile = NULL;
    int history = 1;

    int argc;
    char **argv;
    
    while((line = linenoise(cli_prompt)) != NULL) {
        
        if (line[0] == '\0') {
            continue;
        }
        
        long repeat = 1;
        int skipargs = 0;
        char *endptr = NULL;
        
        argv = cliSplitArgs(line,&argc);

        /* check if we have a repeat command option and
         * need to skip the first arg */
        if (argv && argc > 0) {
            errno = 0;
            repeat = strtol(argv[0], &endptr, 10);
            if (argc > 1 && *endptr == '\0') {
                if (errno == ERANGE || errno == EINVAL || repeat <= 0) {
                    fputs("Invalid redis-cli repeat command option value.\n", stdout);
                    sdsfreesplitres(argv, argc);
                    linenoiseFree(line);
                    continue;
                }
                skipargs = 1;
            } else {
                repeat = 1;
            }
        }

        if (history) linenoiseHistoryAdd(line);
        if (historyfile) linenoiseHistorySave(historyfile);
        
        if (argv == NULL) {
            printf("\t Invalid argument(s)\n");
            linenoiseFree(line);
            continue;
        }
        if (argc > 0) {
            if (strcasecmp(argv[0],"quit") == 0 ||
                strcasecmp(argv[0],"exit") == 0)
            {
                printf("\n\t Goodbye!\n");
                return ;
            } else if (argc == 1 && !strcasecmp(argv[0],"clear")) {
                linenoiseClearScreen();
            } else {
                invoke_command(argc - skipargs, argv + skipargs, repeat);
            }
        }
        /* Free the argument vector */
        sdsfreesplitres(argv,argc);
        /* linenoise() returns malloc-ed lines like readline() */
        linenoiseFree(line);
    }
}

void cli_exit()
{
    //printf("fastlog cli quit.\n");
}
