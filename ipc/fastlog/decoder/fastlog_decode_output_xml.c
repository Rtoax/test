#define _GNU_SOURCE
#include <fastlog_decode.h>


#ifdef FASTLOG_HAVE_LIBXML2

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xpath.h>

#endif /*<FASTLOG_HAVE_LIBXML2>*/


static int xml_open(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_XML)) {
        assert(0 && "Not XML type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_LIBXML2

    if(o->filename) {

    } else {

    }

    o->file_handler.xml.doc = xmlNewDoc(BAD_CAST"1.0");
    o->file_handler.xml.root_node = xmlNewNode(NULL,BAD_CAST"fastlog");

    xmlDocSetRootElement(o->file_handler.xml.doc, o->file_handler.xml.root_node);
    
    o->file_handler.xml.header = xmlNewNode(NULL,BAD_CAST"header");
    o->file_handler.xml.body = xmlNewNode(NULL,BAD_CAST"body");
    o->file_handler.xml.footer = xmlNewNode(NULL,BAD_CAST"footer");

    xmlAddChild(o->file_handler.xml.root_node, o->file_handler.xml.header);
    xmlAddChild(o->file_handler.xml.root_node, o->file_handler.xml.body);
    xmlAddChild(o->file_handler.xml.root_node, o->file_handler.xml.footer);
    
#endif //FASTLOG_HAVE_LIBXML2

    return 0;
}


static int xml_header(struct output_struct *o, struct fastlog_file_header *header)
{    
    if(!(o->file&LOG_OUTPUT_FILE_XML)) {
        assert(0 && "Not XML type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_LIBXML2

    //header->unix_uname.sysname, 
    //header->unix_uname.release, 
    //header->unix_uname.version, 
    //header->unix_uname.machine,
    //header->unix_uname.nodename, 

    
    xmlNodePtr version = xmlNewNode(NULL, BAD_CAST "version");  
    xmlAddChild(o->file_handler.xml.header, version);
    xmlNewProp(version, BAD_CAST"SW",BAD_CAST decoder_config.decoder_version);
    xmlNewProp(version, BAD_CAST"author",BAD_CAST "Rong Tao");

    xmlNodePtr UTS = xmlNewNode(NULL, BAD_CAST "UTS");  
    xmlAddChild(o->file_handler.xml.header, UTS);
    
    xmlNewProp(UTS, BAD_CAST"sysname",BAD_CAST header->unix_uname.sysname);
    xmlNewProp(UTS, BAD_CAST"kernel",BAD_CAST header->unix_uname.release);
    xmlNewProp(UTS, BAD_CAST"version",BAD_CAST header->unix_uname.version);
    xmlNewProp(UTS, BAD_CAST"machine",BAD_CAST header->unix_uname.machine);
    xmlNewProp(UTS, BAD_CAST"nodename",BAD_CAST header->unix_uname.nodename);

    xmlNodePtr timestamp = xmlNewNode(NULL, BAD_CAST "timestamp");  
    xmlAddChild(o->file_handler.xml.header, timestamp);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"    
    {
        char buffer[256] = {0};
        struct tm _tm;
        localtime_r(&header->unix_time_sec, &_tm);
        strftime(buffer, 256, "%Y-%d-%m/%T", &_tm);
        xmlNewProp(timestamp, BAD_CAST"record",BAD_CAST buffer);
    }
#pragma GCC diagnostic pop

    {
        char buffer[256] = {0};
        struct tm _tm;
        time_t _t = time(NULL);
        localtime_r(&_t, &_tm);
        strftime(buffer, 256, "%Y-%m-%d/%T", &_tm);
        
        xmlNewProp(timestamp, BAD_CAST"time-file",BAD_CAST buffer);
    }

    
    o->file_handler.xml.header_metadata = xmlNewNode(NULL,BAD_CAST"metadata");
    xmlAddChild(o->file_handler.xml.header, o->file_handler.xml.header_metadata);
#endif //FASTLOG_HAVE_LIBXML2
    /*  */
    return 0;
}

static int xml_meta_item(struct output_struct *o, struct metadata_decode *meta)
{
    //assert( 0&& "就是玩");
#ifdef FASTLOG_HAVE_LIBXML2

    char* user_string    = meta->user_string; 
    char* src_filename   = meta->src_filename; 
    char* src_function   = meta->src_function; 
    int   src_line       = meta->metadata->log_line; 
    char* thread_name    = meta->thread_name;
    unsigned long log_num= meta->id_cnt;
    
    const char *(*my_strlevel)(enum FASTLOG_LEVEL level);

    /* 文件输出还是不要颜色了 */
    if(o->filename) {
        my_strlevel = strlevel;
    } else {
    /* 终端输出有颜色骚气一点 */
        my_strlevel = strlevel_color;
    }

    
    xmlNodePtr xmlmeta = xmlNewNode(NULL, BAD_CAST "meta");  
    xmlAddChild(o->file_handler.xml.header_metadata, xmlmeta);

    {
        char buffer[16] = {0};
        sprintf(buffer, "%d", meta->log_id);
        xmlNewProp(xmlmeta, BAD_CAST"ID",BAD_CAST buffer);
    }
    
    xmlNewProp(xmlmeta, BAD_CAST"LV",BAD_CAST my_strlevel(meta->metadata->log_level));
    xmlNewProp(xmlmeta, BAD_CAST"NM",BAD_CAST user_string);
    xmlNewProp(xmlmeta, BAD_CAST"FILE",BAD_CAST src_filename);
    
    {
        char buffer[256] = {0};
        sprintf(buffer, "%s:%d", src_function, src_line);
        xmlNewProp(xmlmeta, BAD_CAST"FUNC",BAD_CAST buffer);
    }
    
    xmlNewProp(xmlmeta, BAD_CAST"THR",BAD_CAST thread_name);
    
    {
        char buffer[16] = {0};
        sprintf(buffer, "%ld", log_num);
        xmlNewProp(xmlmeta, BAD_CAST"LOGs",BAD_CAST buffer);
    }

    o->output_meta_cnt ++;
    
#endif  //FASTLOG_HAVE_LIBXML2

    return 0;
}


static int xml_log_item(struct output_struct *o, struct logdata_decode *logdata, char *log)
{
    if(!(o->file&LOG_OUTPUT_FILE_XML)) {
        assert(0 && "Not XML type.");
        return -1;
    }

    
#ifdef FASTLOG_HAVE_LIBXML2

    
    const char *(*my_strlevel)(enum FASTLOG_LEVEL level);

    /* 文件输出还是不要颜色了 */
    if(o->filename) {
        my_strlevel = strlevel;
    } else {
    /* 终端输出有颜色骚气一点 */
        my_strlevel = strlevel_color;
    }
    

    xmlNodePtr log_item = xmlNewNode(NULL, BAD_CAST "log");  
    xmlAddChild(o->file_handler.xml.body, log_item);

    //log ID
    char buffer[32] = {0};
    sprintf(buffer, "%d", logdata->logdata->log_id);
    xmlNewProp(log_item, BAD_CAST"id",BAD_CAST buffer);

    //级别,模块名,函数名,线程名
    xmlNewProp(log_item, BAD_CAST"lv",BAD_CAST my_strlevel(logdata->metadata->metadata->log_level));
    xmlNewProp(log_item, BAD_CAST"nm",BAD_CAST logdata->metadata->user_string);
    xmlNewProp(log_item, BAD_CAST"fn",BAD_CAST logdata->metadata->src_function);
    
    sprintf(buffer, "%d", logdata->metadata->metadata->log_line);
    xmlNewProp(log_item, BAD_CAST"ln",BAD_CAST buffer);
    
    xmlNewProp(log_item, BAD_CAST"thread",BAD_CAST logdata->metadata->thread_name);

    //时间戳
    char timestamp_buf[32] = {0};
    timestamp_tsc_to_string(logdata->logdata->log_rdtsc, timestamp_buf);
    xmlNodePtr timestamp = xmlNewNode(NULL, BAD_CAST "timestamp");  
    xmlAddChild(log_item, timestamp);
    xmlAddChild(timestamp, xmlNewText(BAD_CAST timestamp_buf));
    
    //日志内容
    xmlNodePtr content = xmlNewNode(NULL, BAD_CAST "content");  
    xmlAddChild(log_item, content);
    char *__p = log;
    while(__p[0] != '\0') {
#define xmlIsChar_ch(c)		(((0x9 <= (c)) && ((c) <= 0xa)) || \
                         ((c) == 0xd) || \
                          (0x20 <= (c)))

        if (*__p == '<' || 
            *__p == '>' || 
            *__p == '&' ||
            *__p == '/' || 
            *__p == '#' ||
            *__p == '\n' ||
            *__p == '\t' ||
            *__p == ':') {
            __p[0] = '`';
    	} else if (xmlIsChar_ch(*__p)) {
    	} else {
            printf("wrong char <%s>%c\n", log, *__p);
            assert(0);
    	}
#undef xmlIsChar_ch        
        __p++;
    }
    
    xmlAddChild(content, xmlNewText(BAD_CAST log));


    o->output_log_cnt ++;

#endif //FASTLOG_HAVE_LIBXML2

    return 0;
}

static int xml_footer(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_XML)) {
        assert(0 && "Not XML type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_LIBXML2
    
    const char *(*my_strlevel)(enum FASTLOG_LEVEL level);
    
    if(o->filename) {
        my_strlevel = strlevel;
    } else {
    /* 终端输出有颜色骚气一点 */
        my_strlevel = strlevel_color;
    }

    xmlNodePtr statistics = xmlNewNode(NULL, BAD_CAST "stats");  
    xmlAddChild(o->file_handler.xml.footer, statistics);

    /* 输出各个日志级别的统计数据 */
    char buffer[32] = {0};
    int _ilevel;
    for(_ilevel=FASTLOG_CRIT; _ilevel<=FASTLOG_DEBUG; _ilevel++) {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%ld", level_count(_ilevel));
        xmlNewProp(statistics, BAD_CAST my_strlevel(_ilevel), BAD_CAST buffer);
    }
    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%ld", o->output_meta_cnt);
    xmlNewProp(statistics, BAD_CAST "OutputMeta", BAD_CAST buffer);
    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%ld", o->output_log_cnt);
    xmlNewProp(statistics, BAD_CAST "OutputLog", BAD_CAST buffer);
    
    xmlNodePtr copyright = xmlNewNode(NULL, BAD_CAST "Copyright");  
    xmlAddChild(o->file_handler.xml.footer, copyright);
    xmlNewProp(copyright, BAD_CAST"Co.", BAD_CAST "ICT reserve all right.");
    xmlNewProp(copyright, BAD_CAST"Author", BAD_CAST "RT");
    
#endif //FASTLOG_HAVE_LIBXML2
    return 0;
}

static int xml_close(struct output_struct *o)
{
    if(!(o->file&LOG_OUTPUT_FILE_XML)) {
        assert(0 && "Not XML type.");
        return -1;
    }
    
#ifdef FASTLOG_HAVE_LIBXML2

    if(o->filename) {
        /* 输出至文件 */
        xmlSaveFormatFile(o->filename, o->file_handler.xml.doc, 1);
    } else {
        /* 终端输出 */
#if 0    
        /* 不带缩进的输出 */
        xmlDocDump(stdout, o->file_handler.xml.doc);
#else
        /* 带缩进的输出 */
        xmlChar *membuffer = NULL;
        int size = 1024;
        
        xmlDocDumpFormatMemory(o->file_handler.xml.doc, &membuffer, &size, 1);

        printf("%s\n", membuffer);

        free(membuffer);
#endif        
    }

    xmlFreeDoc(o->file_handler.xml.doc);

    o->file_handler.xml.doc = NULL;
    o->output_log_cnt = 0;
    o->output_meta_cnt = 0;
    
#endif //FASTLOG_HAVE_LIBXML2
    return 0;
}




struct output_operations output_operations_xml = {
    .open = xml_open,
    .header = xml_header,
    .meta_item = xml_meta_item,
    .log_item = xml_log_item,
    .footer = xml_footer,
    .close = xml_close,
};


struct output_struct output_xml = {
#ifdef FASTLOG_HAVE_LIBXML2
    .enable = true,
#else
    .enable = false,
#endif //FASTLOG_HAVE_LIBXML2
    
    .file = LOG_OUTPUT_FILE_XML|LOG_OUTPUT_ITEM_MASK,
    .filename = NULL,
    .file_handler = {
        .xml = {NULL},
    },
    
    .output_meta_cnt = 0,
    .output_log_cnt = 0,
    .ops = &output_operations_xml,
    
    .filter_num = 0,
    .filter = {NULL},
};

