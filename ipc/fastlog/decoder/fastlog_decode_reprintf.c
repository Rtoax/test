
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <fastlog_decode.h>


#define MAX_BUFFER_LEN 1024


const char printf_format_arg_pattern[] = {
    "^%"
    "([-+ #0]+)?"           // Flags (Position 1)
    "([0-9]+|\\*)?"         // Width (Position 2)
    "(\\.([0-9]+|\\*))?"    // Precision (Position 4; 3 includes '.')
    "(hh|h|l|ll|j|z|Z|t|L)?"// Length (Position 5)
    "([diuoxXfFeEgGaAcspn])"// Specifier (Position 6)
};

/**
 *  解析一个 argument
 *
 *  参数
 *  -----------------------------------
 *  buffer      将被写入的buffer内存地址
 *  use_bytes   成功写入的buffer长度
 *  nr_stars    格式化字符串中包含几个`*`号，如`%*.*s`的`nr_stars=2`
 *  arg_fmt     格式化字符串，如"s","%d"等(包括`%*.*s`)
 *  type        格式化字符串的类型
 *                  这里需要注意的是，`%*.*s`标识三个数据类型，int,int,char*,
 *                  那么这里的`type`对应`char*`，剩下两个`int`在`nr_stars`中标记
 *  arg_addr    存储 argument 的内存地址，见`__fastlog_print_buffer`
 *  
 */
// %d + INT + 0x01 ==> 1
static int sprintf_arg(char *buffer, int *use_bytes, int nr_stars, char *arg_fmt, enum format_arg_type type, void *arg_addr)
{
    int ret = 0;
    char *p_arg = (char *)arg_addr;

    *use_bytes = 0;
    
    switch(type) {

#define _CASE(fat_type, type)                       \
    case fat_type: {                                \
        switch(nr_stars) {                          \
        case 0: {                                   \
            type _val = *(type*)p_arg;              \
            *use_bytes = sprintf(buffer, arg_fmt, _val);         \
            break;                                  \
        }                                           \
        case 1: {                                   \
            int _width = *(int*)p_arg;              \
            p_arg += sizeof(int);                   \
            ret += sizeof(int);                     \
            type _val = *(type*)p_arg;              \
            *use_bytes = sprintf(buffer, arg_fmt, _width, _val); \
            break;                                  \
        }                                           \
        case 2: {                                   \
            int _width = *(int*)p_arg;              \
            p_arg += sizeof(int);                   \
            int _len = *(int*)p_arg;                \
            p_arg += sizeof(int);                   \
            ret += 2*sizeof(int);                   \
            type _val = *(type*)p_arg;              \
            *use_bytes = sprintf(buffer, arg_fmt, _width,_len, _val);    \
            break;                                  \
        }                                           \
        default: {                                  \
            assert(0 && "Not support stars > 2");   \
            break;                                  \
        }                                           \
        }                                           \
        ret += SIZEOF_FAT(fat_type);                \
        break;                                      \
    }

#if 0
#define _CASE_STRING(fat_type, type)            \
    case fat_type: {                            \
        type _val = (type)arg_addr;             \
        /*printf("STR:%s\n", _val); */          \
        printf(arg_fmt, _val);                  \
        ret += strlen(_val) + 1;                \
        break;                                  \
    }
#else
#define _CASE_STRING(fat_type, type)                \
    case fat_type: {                                \
        type _val;                                  \
        switch(nr_stars) {                          \
        case 0: {                                   \
            _val = (type)p_arg;                     \
            /*printf("STR:%s\n", _val); */          \
            *use_bytes = sprintf(buffer, arg_fmt, _val);         \
            break;                                  \
        }                                           \
        case 1: {                                   \
            int _width = *(int*)p_arg;              \
            p_arg += sizeof(int);                   \
            ret += sizeof(int);                     \
            _val = (type)p_arg;                     \
            /*printf("STR:%s\n", _val); */          \
            *use_bytes = sprintf(buffer, arg_fmt, _width, _val); \
            break;                                  \
        }                                           \
        case 2: {                                   \
            int _width = *(int*)p_arg;              \
            p_arg += sizeof(int);                   \
            int _len = *(int*)p_arg;                \
            p_arg += sizeof(int);                   \
            ret += 2*sizeof(int);                   \
            _val = (type)p_arg;                     \
            /*printf("STR:%s\n", _val); */          \
            *use_bytes = sprintf(buffer, arg_fmt, _width, _len, _val);    \
            break;                                  \
        }                                           \
        default: {                                  \
            assert(0 && "Not support stars > 2");   \
            break;                                  \
        }                                           \
        }                                           \
        ret += strlen(_val) + 1;                    \
        break;                                      \
    }
#endif
        _CASE(FAT_INT8, int8_t);
        _CASE(FAT_INT16, int16_t);
        _CASE(FAT_INT32, int32_t);
        _CASE(FAT_INT64, int64_t);
        
        _CASE(FAT_UINT8, uint8_t);
        _CASE(FAT_UINT16, uint16_t);
        _CASE(FAT_UINT32, uint32_t);
        _CASE(FAT_UINT64, uint64_t);
        
        _CASE(FAT_INT, int);
        _CASE(FAT_SHORT, short);
        _CASE(FAT_SHORT_INT, short int);

        _CASE(FAT_LONG, long);
        _CASE(FAT_LONG_INT, long int);
        _CASE(FAT_LONG_LONG, long long);
        _CASE(FAT_LONG_LONG_INT, long long int);

        _CASE(FAT_CHAR, char);
        _CASE(FAT_UCHAR, unsigned char);
        
        _CASE_STRING(FAT_STRING, char *);

        _CASE(FAT_POINTER, void*);
        
        _CASE(FAT_FLOAT, double);
        _CASE(FAT_DOUBLE, double);
        _CASE(FAT_LONG_DOUBLE, long double);
        
        default:
            printf("\t unknown(%d).\n", type);
            assert(0 && "Not support type." && __FILE__ && __LINE__);
            break;
#undef _CASE
#undef _CASE_STRING
    }

    return ret;
}



static void sprintf_regex_format(char *buffer, char* format, struct args_type *argsType, fastlog_logdata_t *log)
{
    int pos_buffer = 0;

    int iarg = 0;
	int status = 0, i = 0;
	int flag = REG_EXTENDED;
	regmatch_t pmatch[1];
	const size_t nmatch = 1;
	regex_t reg;
    
	const char *pattern = printf_format_arg_pattern;

    char *args_addr = log->log_args_buff;

    regcomp(&reg, pattern, flag);

    /* 剩下的格式化字符串长度 */
    int len_fmt_remain = strlen(format);
    char *fmt = format;

    int p_fmt = 0;
    
    while(len_fmt_remain>0) {
        
        /**
         *  N_stars 用于带星号的格式化输出 `%*.*s`
         *
         *  `%*.*s` -> N_stars = 2
         *  `%.*s`  -> N_stars = 1
         *  `%s`    -> N_stars = 0
         *
         *  意在解决如下的格式化输出
         *
         *  -------------------------------------------
         *  printf("%.*s\n", 4, "hello");
         *  printf("%*s\n", 40, "hell0");
         *  printf("%*.*s\n", 40, 4, "hell0");
         *  输出为：
         *  hell
         *                                    hell0
         *                                     hell
         *
         *  整形略有不同
         *  -------------------------------------------
         *  printf("%.*d\n", 4, 10000);
         *  printf("%*d\n", 40, 10000);
         *  printf("%*.*d\n", 40, 4, 10000);
         *  输出为：
         *  10000
         *                                    10000
         *                                    10000
         */
        int N_stars;

        /**
         *  buf_use_bytes - `printf_arg`写入到 buffer 的字节数
         *
         *  将造成`pos_buffer`位置的更新
         */
        int buf_use_bytes = 0;

        /**
         *  保存从 format 中解析出来的 格式化 % 字符串
         *  
         *  如：
         *  format = "Hello, %s, I'm %d years young."
         *  解析出的 `internal_fmt` 分别为：
         *  "%s", "%d"
         */
        char internal_fmt[8] = {0};
        
        i = 1;
    
    	status = regexec(&reg, fmt, nmatch, pmatch, 0);
        
    	if(status == REG_NOMATCH) {

            buffer[pos_buffer++] = fmt[0];
            
    	} else if(status == 0) {
    	
            N_stars = 0;

            /**
             * 正则化获取格式化字符串
             *
             * 起始时 fmt = "Hello, %s, my firend."
             *  
             *  若正则表达式匹配成功：
             *  fmt = "%s, my firend."
             *  pmatch = "%s"
             *  internal_fmt = "%s"
             *  N_stars = 0;
             */
            for(i = pmatch[0].rm_so; i < pmatch[0].rm_eo; i++) {
                internal_fmt[i] = fmt[i];
                if(internal_fmt[i] == '*') {
                    N_stars ++;
                }
    		}
            internal_fmt[i] = '\0';

            iarg += N_stars;

            buf_use_bytes = 0;
            
            args_addr += sprintf_arg(&buffer[pos_buffer], &buf_use_bytes, N_stars, internal_fmt, argsType->argtype[iarg], args_addr);
//            printf("%d->%d(%s)\n", iarg, argsType->argtype[iarg], FASTLOG_FAT_TYPE2SIZENAME[argsType->argtype[iarg]].name);
            pos_buffer += buf_use_bytes;
            
            iarg ++;
    	}
        
        fmt += i;
        len_fmt_remain -= i;
        
    }
    
	regfree(&reg);
}


int reprintf(struct logdata_decode *logdata, struct output_struct *output)
{
    char log_buffer[MAX_BUFFER_LEN] = {0};

    sprintf_regex_format(log_buffer, logdata->metadata->print_format, &logdata->metadata->argsType, logdata->logdata);
    
    output_log_item(output, logdata, log_buffer);
}

