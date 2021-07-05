#define _GNU_SOURCE
#include <fastlog_decode.h>


/**
 *  JSON 输出的支持
 *  荣涛 2021年6月25日15:43:28
 */

#ifdef FASTLOG_HAVE_JSON
#include <json/json.h>
#endif


static int json_open(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_JSON)) {
        assert(0 && "Not JSON type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_JSON

    o->file_handler.json.header = json_object_new_object();
    o->file_handler.json.metadata = json_object_new_object();
    o->file_handler.json.logdata = json_object_new_object();
    o->file_handler.json.footer = json_object_new_object();

    if(o->filename) {
        progress_reset(&pro_bar, o->filename);
        
        o->file_handler.json.root = json_object_new_object();
        json_object_object_add(o->file_handler.json.root, "header", o->file_handler.json.header);
        json_object_object_add(o->file_handler.json.root, "metadata", o->file_handler.json.metadata);
        json_object_object_add(o->file_handler.json.root, "logdata", o->file_handler.json.logdata);
        json_object_object_add(o->file_handler.json.root, "footer", o->file_handler.json.footer);
    }


#endif //FASTLOG_HAVE_JSON

    return 0;
}


static int json_header(struct output_struct *o, struct fastlog_file_header *header)
{    
    if(!(o->file&LOG_OUTPUT_FILE_JSON)) {
        assert(0 && "Not JSON type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_JSON

    json_object *log_header = o->file_handler.json.header;

    json_object_object_add(log_header, "author", json_object_new_string("Rong Tao"));
    json_object_object_add(log_header, "version", json_object_new_string(decoder_config.decoder_version));
    json_object_object_add(log_header, "author", json_object_new_string("Rong Tao"));

    json_object *uts = json_object_new_object();
    
    json_object_object_add(log_header, "UTS", uts);

    json_object_object_add(uts, "sysname",json_object_new_string( header->unix_uname.sysname));
    json_object_object_add(uts, "kernel",json_object_new_string( header->unix_uname.release));
    json_object_object_add(uts, "version",json_object_new_string( header->unix_uname.version));
    json_object_object_add(uts, "machine",json_object_new_string( header->unix_uname.machine));
    json_object_object_add(uts, "nodename",json_object_new_string( header->unix_uname.nodename));

#if defined (__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
    {
        char buffer[256] = {0};
        struct tm _tm;
        localtime_r(&header->unix_time_sec, &_tm);
        strftime(buffer, 256, "%Y-%m-%d/%T", &_tm);
        
        json_object_object_add(log_header, "record",json_object_new_string( buffer));
    }
#if defined (__GNUC__) && (__GNUC__ >= 7)
#pragma GCC diagnostic pop
#endif
    {
        char buffer[256] = {0};
        struct tm _tm;
        time_t _t = time(NULL);
        localtime_r(&_t, &_tm);
        strftime(buffer, 256, "%Y-%m-%d/%T", &_tm);
        
        json_object_object_add(log_header, "time-file",json_object_new_string( buffer));
    }

    if(!o->filename) {
        printf("%s\r\n", json_object_to_json_string_ext(log_header, JSON_C_TO_STRING_PRETTY));
    }
    
    //json_object_to_file_ext("json.out.json", log_header, JSON_C_TO_STRING_PRETTY);

#endif //FASTLOG_HAVE_JSON

    /*  */
    return 0;
}

static int json_meta_item(struct output_struct *o, struct metadata_decode *meta)
{
    //assert( 0&& "就是玩");
#ifdef FASTLOG_HAVE_JSON

    char* user_string    = meta->user_string; 
    char* src_filename   = meta->src_filename; 
    char* src_function   = meta->src_function; 
    int   src_line       = meta->metadata->log_line; 
    char* thread_name    = meta->thread_name;
    unsigned long log_num= meta->id_cnt;
    
    char jsonmetabuffer[256] = {0};
    char funcbuffer[256] = {0};


    json_object *jsonmeta = json_object_new_object();
    
    json_object_object_add(jsonmeta, "ID", json_object_new_int(meta->log_id));
    json_object_object_add(jsonmeta, "LV", json_object_new_string(strlevel(meta->metadata->log_level)));
    json_object_object_add(jsonmeta, "NM", json_object_new_string(user_string));
    json_object_object_add(jsonmeta, "FILE", json_object_new_string(src_filename));
    

    sprintf(funcbuffer, "%s:%d", src_function, src_line);
    json_object_object_add(jsonmeta, "FILE", json_object_new_string(funcbuffer));

    json_object_object_add(jsonmeta, "THR", json_object_new_string(thread_name));
    json_object_object_add(jsonmeta, "LOGs", json_object_new_int(log_num));




    if(!o->filename) {
        printf("%s\r\n", json_object_to_json_string_ext(jsonmeta, JSON_C_TO_STRING_PRETTY));
        //json_object_object_del(struct json_object * obj, const char * key);
        json_object_put(jsonmeta);
    } else {
        sprintf(jsonmetabuffer, "meta:%ld", o->output_meta_cnt);
        json_object_object_add(o->file_handler.json.metadata, jsonmetabuffer, jsonmeta);
    }

    o->output_meta_cnt ++;

#endif  //FASTLOG_HAVE_JSON

    return 0;
}


static int json_log_item(struct output_struct *o, struct logdata_decode *logdata, char *log)
{
    if(!(o->file&LOG_OUTPUT_FILE_JSON)) {
        assert(0 && "Not JSON type.");
        return -1;
    }

    
#ifdef FASTLOG_HAVE_JSON

    char loglabel_string[256] = {0};

    json_object *loglabel = json_object_new_object();

    //log ID
    json_object_object_add(loglabel, "id", json_object_new_int(logdata->logdata->log_id));

    //级别,模块名,函数名,线程名
    json_object_object_add(loglabel, "lv", json_object_new_string(strlevel(logdata->metadata->metadata->log_level)));
    json_object_object_add(loglabel, "nm", json_object_new_string(logdata->metadata->user_string));
    json_object_object_add(loglabel, "fn", json_object_new_string(logdata->metadata->src_function));
    json_object_object_add(loglabel, "ln", json_object_new_int(logdata->metadata->metadata->log_line));
    json_object_object_add(loglabel, "thread", json_object_new_string(logdata->metadata->thread_name));

    

    //时间戳
    char timestamp_buf[32] = {0};
    timestamp_tsc_to_string(logdata->logdata->log_rdtsc, timestamp_buf);
    json_object_object_add(loglabel, "time", json_object_new_string(timestamp_buf));
    
    //日志内容
    json_object_object_add(loglabel, "content", json_object_new_string(log));


    if(!o->filename) {
        printf("%s\r\n", json_object_to_json_string_ext(loglabel, JSON_C_TO_STRING_PRETTY));
        //json_object_object_del(struct json_object * obj, const char * key);
        json_object_put(loglabel);
    } else {
        sprintf(loglabel_string, "log:%ld", o->output_log_cnt);
        json_object_object_add(o->file_handler.json.logdata, loglabel_string, loglabel);
    }


    o->output_log_cnt ++;

    
#endif //FASTLOG_HAVE_JSON

    return 0;
}

static int json_footer(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_JSON)) {
        assert(0 && "Not JSON type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_JSON
    
    json_object *log_footer = o->file_handler.json.footer;

    json_object *stats_footer = json_object_new_object();

    /* 输出各个日志级别的统计数据 */
    int _ilevel;
    for(_ilevel=FASTLOG_CRIT; _ilevel<=FASTLOG_DEBUG; _ilevel++) {
        json_object_object_add(stats_footer, strlevel(_ilevel), json_object_new_int64(level_count(_ilevel)));
    }

    json_object_object_add(stats_footer, "OutputMeta", json_object_new_int64(o->output_meta_cnt));
    json_object_object_add(stats_footer, "OutputMetaEx", json_object_new_int64(meta_hdr()->data_num));
    json_object_object_add(stats_footer, "OutputLog", json_object_new_int64(o->output_log_cnt));
    json_object_object_add(stats_footer, "OutputLogEx", json_object_new_int64(decoder_config.total_flog_num));
    
    json_object_object_add(stats_footer, "Co.", json_object_new_string("ICT reserve all right."));
    json_object_object_add(stats_footer, "Author", json_object_new_string("RT"));


    if(!o->filename) {
        printf("%s\r\n", json_object_to_json_string_ext(stats_footer, JSON_C_TO_STRING_PRETTY));
        json_object_put(stats_footer);
    } else {
        json_object_object_add(log_footer, "stats", stats_footer);
    }
    
#endif //FASTLOG_HAVE_JSON
    return 0;
}

static int json_close(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_JSON)) {
        assert(0 && "Not JSON type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_JSON


    if(o->filename) {
        json_object_to_file_ext(o->filename, o->file_handler.json.root, JSON_C_TO_STRING_PRETTY);
        //printf("%s\r\n", json_object_to_json_string_ext(o->file_handler.json.root, JSON_C_TO_STRING_PRETTY));
    }

    //释放

    if(o->filename) {
        json_object_put(o->file_handler.json.root);

    } else {
    
        json_object_put(o->file_handler.json.header);
        json_object_put(o->file_handler.json.metadata);
        json_object_put(o->file_handler.json.logdata);
        json_object_put(o->file_handler.json.footer);
    }
    
    o->file_handler.json.header = NULL;
    o->file_handler.json.metadata = NULL;
    o->file_handler.json.logdata = NULL;
    o->file_handler.json.footer = NULL;
    o->file_handler.json.root = NULL;
    
#endif //FASTLOG_HAVE_JSON
    
    o->output_log_cnt = 0;
    o->output_meta_cnt = 0;

    return 0;
}




struct output_operations output_operations_json = {
    .open = json_open,
    .header = json_header,
    .meta_item = json_meta_item,
    .log_item = json_log_item,
    .footer = json_footer,
    .close = json_close,
};



struct output_struct output_json = {
#ifdef FASTLOG_HAVE_JSON
    .enable = true,
#else
    .enable = false,
#endif //FASTLOG_HAVE_JSON
    
    .file = LOG_OUTPUT_FILE_JSON|LOG_OUTPUT_ITEM_MASK,
    .filename = NULL,
    .file_handler = {
        .json = {NULL},
    },
    
    .output_meta_cnt = 0,
    .output_log_cnt = 0,
    .ops = &output_operations_json,
    
    .filter_num = 0,
    .filter = {NULL},
};


