#define _GNU_SOURCE
#include <fastlog_decode.h>


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
    o->file_handler.json.header = json_object_new_array();

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

//    TODO
    
//    json_object_object_add(log_header, "version", json_object_new_string(decoder_config.decoder_version));
//    json_object_object_add(log_header, "author", json_object_new_string("Rong Tao"));

//    json_object *uts = json_object_new_array();
//    
//    json_object_object_add(log_header, "UTS", uts);
//
//    json_object_object_add(uts, "sysname",json_object_new_string( header->unix_uname.sysname));
//    json_object_object_add(uts, "kernel",json_object_new_string( header->unix_uname.release));
//    json_object_object_add(uts, "version",json_object_new_string( header->unix_uname.version));
//    json_object_object_add(uts, "machine",json_object_new_string( header->unix_uname.machine));
//    json_object_object_add(uts, "nodename",json_object_new_string( header->unix_uname.nodename));
    
//    {
//        char buffer[256] = {0};
//        struct tm _tm;
//        localtime_r(&header->unix_time_sec, &_tm);
//        strftime(buffer, 256, "%Y-%d-%m/%T", &_tm);
//        
//        json_object_object_add(log_header, "record",json_object_new_string( buffer));
//    }
//    {
//        char buffer[256] = {0};
//        struct tm _tm;
//        time_t _t = time(NULL);
//        localtime_r(&_t, &_tm);
//        strftime(buffer, 256, "%Y-%m-%d/%T", &_tm);
//        
//        json_object_object_add(log_header, "time-file",json_object_new_string( buffer));
//    }


//    o->file_handler.json.metadata = json_object_new_array();
//    json_object_object_add(header, "metadata", o->file_handler.json.metadata);

    printf("\tResult:%s\r\n", json_object_to_json_string(log_header));

    json_object_to_file("json.out", log_header);

#endif //FASTLOG_HAVE_JSON

    /*  */
    return 0;
}

static int json_meta_item(struct output_struct *o, struct metadata_decode *meta)
{
    //assert( 0&& "就是玩");
#ifdef FASTLOG_HAVE_JSON

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

#endif //FASTLOG_HAVE_JSON
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


