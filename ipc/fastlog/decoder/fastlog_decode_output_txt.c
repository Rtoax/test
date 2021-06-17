#include <fastlog_decode.h>


static int txt_open(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_TXT)) {
        assert(0 && "Not txt type.");
        return -1;
    }
    
    if(o->filename) {
        o->file_handler.fp = fopen(o->filename, "w");
    } else {
        o->file_handler.fp = stdout;
    }
        
}


static int txt_header(struct output_struct *o, struct fastlog_file_header *header)
{    
    if(!(o->file&LOG_OUTPUT_FILE_TXT)) {
        assert(0 && "Not txt type.");
        return -1;
    }
    
    fprintf(o->file_handler.fp, "This is FastLog LOG file.\n");

    /* UTS */
	fprintf(o->file_handler.fp, 
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
        strftime(buffer, 256, "This log recoreded in:  %Y-%d-%m/%T", &_tm);
        fprintf(o->file_handler.fp, "%s\n",buffer);
    }
    {
        char buffer[256] = {0};
        struct tm _tm;
        time_t _t = time(NULL);
        localtime_r(&_t, &_tm);
        strftime(buffer, 256, "This file create in:  %Y-%m-%d/%T", &_tm);
        fprintf(o->file_handler.fp, "%s\n", buffer);
    }

    fprintf(o->file_handler.fp, "\n");
    fprintf(o->file_handler.fp, "The LOGs as follows...\n");
    fprintf(o->file_handler.fp, "--------------------------------------------\n");
    
    /*  */
    return 0;
}


static int txt_log_item(struct output_struct *o, struct logdata_decode *logdata, char *log)
{
    if(!(o->file&LOG_OUTPUT_FILE_TXT)) {
        assert(0 && "Not txt type.");
        return -1;
    }
    
    char* user_string    = logdata->metadata->user_string; 
    char* src_filename   = logdata->metadata->src_filename; 
    char* src_function   = logdata->metadata->src_function; 
    int   src_line       = logdata->metadata->metadata->log_line; 
    char* print_format   = logdata->metadata->print_format;
    char* thread_name    = logdata->metadata->thread_name;

    //时间戳
    char timestamp_buf[32] = {0};
    timestamp_tsc_to_string(logdata->logdata->log_rdtsc, timestamp_buf);

    const char *(*my_strlevel)(enum FASTLOG_LEVEL level);

    /* 文件输出还是不要颜色了 */
    if(o->filename) {
        my_strlevel = strlevel;
    } else {
    /* 终端输出有颜色骚气一点 */
        my_strlevel = strlevel_color;
    }

    fprintf(o->file_handler.fp, "[%s][%s][%s:%d][%s] ", 
                                my_strlevel(logdata->metadata->metadata->log_level), 
                                timestamp_buf,
                                src_function, 
                                src_line, 
                                thread_name);
    
    fprintf(o->file_handler.fp, "%s", log);
    
}

static int txt_footer(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_TXT)) {
        assert(0 && "Not txt type.");
        return -1;
    }
    fprintf(o->file_handler.fp, "\n");

    
    const char *(*my_strlevel)(enum FASTLOG_LEVEL level);
    char *level_fmt = NULL;
    
    /* 文件输出还是不要颜色了 */
    if(o->filename) {
        my_strlevel = strlevel;
        level_fmt = "        %-8s %-8s %-8s %-8s %-8s \n";
    } else {
    /* 终端输出有颜色骚气一点 */
        my_strlevel = strlevel_color;
        level_fmt = "        %-19s %-19s %-19s %-19s %-19s \n";
    }


        fprintf(o->file_handler.fp, level_fmt, 
                                     my_strlevel(FASTLOG_CRIT), 
                                     my_strlevel(FASTLOG_ERR),
                                     my_strlevel(FASTLOG_WARNING),
                                     my_strlevel(FASTLOG_INFO),
                                     my_strlevel(FASTLOG_DEBUG));


                                                                     
    fprintf(o->file_handler.fp, "Number  %-8ld %-8ld %-8ld %-8ld %-8ld \n", 
                                                                    level_count(FASTLOG_CRIT),
                                                                    level_count(FASTLOG_ERR),
                                                                    level_count(FASTLOG_WARNING),
                                                                    level_count(FASTLOG_INFO),
                                                                    level_count(FASTLOG_DEBUG));
    fprintf(o->file_handler.fp, "\n");

    fprintf(o->file_handler.fp, "Total metas %ld\n", meta_count());
    fprintf(o->file_handler.fp, "Total logs  %ld\n", log_count());

    fprintf(o->file_handler.fp, "txt output done.\n");

    return 0;
}

static int txt_close(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_TXT)) {
        assert(0 && "Not txt type.");
        return -1;
    }
    
    if(o->filename) {
        close(o->file_handler.fp);
    }
    o->file_handler.fp = NULL;

    return 0;
}

struct output_operations output_operations_txt = {
    .open = txt_open,
    .header = txt_header,
    .log_item = txt_log_item,
    .footer = txt_footer,
    .close = txt_close,
};

struct output_struct output_txt = {
    .range = LOG__RANGE_ALL,

    .range_value = {
        .level = 0,
        .name = NULL,
    },
    
    .file = LOG_OUTPUT_FILE_TXT|LOG_OUTPUT_FILE_CONSOLE,
    .filename = NULL,
    .file_handler = {
        .fp = NULL,
    },
    .ops = &output_operations_txt,
};
