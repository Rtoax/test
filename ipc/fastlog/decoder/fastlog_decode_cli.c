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

static char *command_groups[] = {
    "show",
    "save",
    "load",
};

enum {
    CMD_SHOW_LEVEL,
};
struct command_help command_helps[] = {
    
    [CMD_SHOW_LEVEL] = { 
        .name = "show",
        .params = "level all|crit|err|warn|info|debug txt|xml|json [FILENAME]",
        .summary = "show log by level and save to txt,xml,json FILENAME",
        .group = GRP_SHOW,
    },
};

char cli_prompt[64]  ={"fastlog>> "};


static void show_help()
{
    printf("`show help`: show this information.\n");
    printf("`%s %s`: %s\n", 
                        command_helps[CMD_SHOW_LEVEL].name, 
                        command_helps[CMD_SHOW_LEVEL].params, 
                        command_helps[CMD_SHOW_LEVEL].summary);
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

static void cmd_show_level(enum FASTLOG_LEVEL log_level, LOG_OUTPUT_FILE_TYPE file_type, char * filename)
{
    char *open_filename = NULL;
    if(filename) {
        open_filename = filename;
    }
    
    struct output_struct *output = NULL;
    
    if(file_type & LOG_OUTPUT_FILE_TXT) {
        output = &output_txt;
    } else if(file_type & LOG_OUTPUT_FILE_XML) {
        output = &output_txt;
    } else if(file_type & LOG_OUTPUT_FILE_JSON) {
        output = &output_txt;
    } else {
        printf("just support txt, xml, json.\n");
        return;
    }
    
    output_open(output, open_filename);
    
    output_header(output, meta_hdr());

    if(log_level >= FASTLOG_CRIT && log_level < FASTLOGLEVELS_NUM) {
        level_list__iter(log_level, logdata_print_output, output);
    } else {
        logdata_rbtree__iter(logdata_print_output, output);
    }
    
    output_footer(output);
    
    output_close(output);

    printf("show level %s 0x%04x %s.\n", strlevel_color(log_level), file_type, filename?filename:"null");
}

static int invoke_command(int argc, char **argv, long repeat)
{
    int i;
    
    //show 
    if(strncasecmp(argv[0], "show", 4) == 0) {
        if(argc == 1) {
            printf("input `show help` to check show command.\n");
            return 0;
        } else if(argc >= 2) {
            //show help
            if(strncasecmp(argv[1], "help", 4) == 0) {
                show_help();
                return 0;
            //show level
            } else if(strncasecmp(argv[1], "level", 5) == 0) {

                /*all|crit|err|warn|info|debug txt|xml|json [FILENAME] */
                enum FASTLOG_LEVEL log_level = FASTLOG_ERR;
                LOG_OUTPUT_FILE_TYPE file_type = LOG_OUTPUT_FILE_TXT;
                char *filename = NULL;
                
                /* 日志级别 */
                if(argc >= 3) {
                    if(strncasecmp(argv[2], "all", 3) == 0) {
                        log_level = FASTLOGLEVEL_ALL;
                    } else if(strncasecmp(argv[2], "crit", 4) == 0) {
                        log_level = FASTLOG_CRIT;
                    } else if(strncasecmp(argv[2], "err", 3) == 0) {
                        log_level = FASTLOG_ERR;
                    } else if(strncasecmp(argv[2], "warn", 4) == 0) {
                        log_level = FASTLOG_WARNING;
                    } else if(strncasecmp(argv[2], "info", 4) == 0) {
                        log_level = FASTLOG_INFO;
                    } else if(strncasecmp(argv[2], "debug", 5) == 0) {
                        log_level = FASTLOG_DEBUG;
                    } else {
                        printf("show level MUST be one of all|crit|err|warn|info|debug.\n");
                        return 0;
                    }
                }
                
                /* 文件格式 */
                if(argc >= 4) {
                    if(strncasecmp(argv[3], "txt", 3) == 0) {
                        file_type = LOG_OUTPUT_FILE_TXT;
                    } else if(strncasecmp(argv[3], "xml", 3) == 0) {
                        file_type = LOG_OUTPUT_FILE_XML;
                    } else if(strncasecmp(argv[3], "json", 4) == 0) {
                        file_type = LOG_OUTPUT_FILE_JSON;
                    } else {
                        printf("show file type MUST be one of txt|xml|json.\n");
                        return 0;
                    }
                }
                
                /* 文件名 */
                if(argc >= 5) {
                    filename = argv[4];
                    if(access(filename, F_OK) == 0) {
                        printf("\t `%s` already exist.\n", filename);
                        return 0;
                    }
                } else {
                    // 未设置文件名，默认向 console 输出
                    file_type |= LOG_OUTPUT_FILE_CONSOLE;
                }

                cmd_show_level(log_level, file_type, filename);
                
            }
        }
        
    } 
    
    
    return 0;
}


void cli_loop()
{
    char *line;
    sds historyfile = NULL;
    int history = 1;
    char *prgname = "fastlog.decoder";

    int argc;
    char **argv;

    int cli_argc;
    sds *cli_argv;
    
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
            printf("Invalid argument(s)\n");
            linenoiseFree(line);
            continue;
        }
        if (argc > 0) {
            if (strcasecmp(argv[0],"quit") == 0 ||
                strcasecmp(argv[0],"exit") == 0)
            {
                printf("\nGoodbye!\n");
                exit(0);
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
