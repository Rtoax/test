#include <fastlog_decode.h>


static bool match_name_ok(struct output_filter *filter, struct logdata_decode *logdata, char *value)
{
    if(filter->log_range & LOG__RANGE_NAME) {
        if(!strstr(logdata->metadata->user_string, value)) {
            return false;
        }
    }
    return true;
}

static bool match_func_ok(struct output_filter *filter, struct logdata_decode *logdata, char *value)
{
    if(filter->log_range & LOG__RANGE_FUNC) {
        if(!strstr(logdata->metadata->src_function, value)) {
            return false;
        }
    }
    
    return true;
}

static bool match_thread_ok(struct output_filter *filter, struct logdata_decode *logdata, char *value)
{
    if(filter->log_range & LOG__RANGE_THREAD) {
        if(!strstr(logdata->metadata->thread_name, value)) {
            return false;
        }
    }
    return true;
}

static bool match_log_content_ok(struct output_filter *filter, struct logdata_decode *logdata, char *log, char *value)
{
    if(filter->log_range & LOG__RANGE_CONTENT) {
        if(!strstr(log, value)) {
            return false;
        }
    }
    return true;
}


const struct output_filter filter_name = {
    .log_range = LOG__RANGE_NAME,
    .filter_type = LOG__FILTER_MATCH_STRSTR,
    .match_prefix_ok = match_name_ok,
};
const struct output_filter filter_func = {
    .log_range = LOG__RANGE_FUNC,
    .filter_type = LOG__FILTER_MATCH_STRSTR,
    .match_prefix_ok = match_func_ok,
};
    
const struct output_filter filter_thread = {
    .log_range = LOG__RANGE_THREAD,
    .filter_type = LOG__FILTER_MATCH_STRSTR,
    .match_prefix_ok = match_thread_ok,
};  
const struct output_filter filter_content = {
    .log_range = LOG__RANGE_CONTENT,
    .filter_type = LOG__FILTER_MATCH_STRSTR,
    .match_log_content_ok = match_log_content_ok,
};


