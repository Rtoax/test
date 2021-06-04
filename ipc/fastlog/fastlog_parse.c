#include <fastlog.h>
#include <regex.h>




/* 从 "%d %s %f %llf" 到 `struct fastlog_print_args`的转化 */
int __fastlog_parse_format(const char *fmt, struct args_type *args)
{
    int iarg;
    int __argtype[_FASTLOG_MAX_NR_ARGS];
    args->nargs = parse_printf_format(fmt, _FASTLOG_MAX_NR_ARGS, __argtype);
    
    for(iarg=0; iarg<args->nargs; iarg++) {
        
        switch(__argtype[iarg]) {

#define _CASE(i, v_from, v_to)                  \
        case v_from:                            \
            args->argtype[i] = v_to;      \
            break;  

        _CASE(iarg, PA_INT, FAT_INT);
        _CASE(iarg, PA_INT|PA_FLAG_LONG, FAT_LONG_INT);
        _CASE(iarg, PA_INT|PA_FLAG_SHORT, FAT_SHORT);
        _CASE(iarg, PA_INT|PA_FLAG_LONG_LONG, FAT_LONG_LONG_INT);

        _CASE(iarg, PA_CHAR, FAT_CHAR);
        _CASE(iarg, PA_WCHAR, FAT_SHORT);   //wchar_t -> short
        _CASE(iarg, PA_STRING, FAT_STRING);
        _CASE(iarg, PA_POINTER, FAT_POINTER);
        _CASE(iarg, PA_FLOAT, FAT_FLOAT);
        _CASE(iarg, PA_DOUBLE, FAT_DOUBLE);
        _CASE(iarg, PA_DOUBLE|PA_FLAG_LONG_DOUBLE, FAT_LONG_DOUBLE);
        
#undef _CASE

        default:
            printf("\t%d unknown(%d).\n", iarg, __argtype[iarg]);
            assert(0 && "Not support wstring type.");
            break;
        }

        
    }
    
    return args->nargs;
}

static void write_buffer(char *buf, size_t size)
{
    char buffer[1024];
    memcpy(buffer, buf, size);
    //TODO
    //这样做的吞吐量低于 NanoLog
}

int __fastlog_print_parse_buffer(char *buffer, struct args_type *args)
{
    int iarg;
    uint64_t log_rdtsc;
    
    struct arg_hdr *arghdr = buffer;

    int log_id = arghdr->log_id;
    log_rdtsc = arghdr->log_rdtsc;
    
    buffer = arghdr->log_args_buff;

    
    for(iarg=0; iarg < args->nargs; iarg++) {

//        printf("parse: %d->%d(%s)\n", iarg, args->argtype[iarg], FASTLOG_FAT_TYPE2SIZENAME[args->argtype[iarg]].name);
        
        switch(args->argtype[iarg]) {

            //恢复数据
            

        }
    }
    write_buffer(buffer, arghdr->log_args_size);

    //这里与 test-0 中的 如下 LOG 相对应
    // FAST_LOG(FASTLOG_WARNING, "TEST", "I have an integer %d", total_dequeue);
    unsigned long *total = (unsigned long *)arghdr->log_args_buff;
    printf("total = %ld\n", *total);

    return 0;
}


