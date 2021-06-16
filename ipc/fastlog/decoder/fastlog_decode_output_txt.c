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
    

    fprintf(o->file_handler.fp, "[%s][%s][%s:%d][%s] ", 
            timestamp_buf,
            FASTLOG_LEVEL_NAME[logdata->metadata->metadata->log_level], src_function, src_line, thread_name);
    
    fprintf(o->file_handler.fp, "%s", log);
    
}

static int txt_footer(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_TXT)) {
        assert(0 && "Not txt type.");
        return -1;
    }
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
