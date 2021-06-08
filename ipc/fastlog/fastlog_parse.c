#include <fastlog.h>
#include <regex.h>




/* 从 "%d %s %f %llf" 到 `struct fastlog_print_args`的转化 */
int __fastlog_parse_format(const char *fmt, struct args_type *args)
{
    int iarg;
    int __argtype[_FASTLOG_MAX_NR_ARGS];
    args->nargs = parse_printf_format(fmt, _FASTLOG_MAX_NR_ARGS, __argtype);
//    printf("args->nargs = %d, fmt = %s, %d\n", args->nargs, fmt, __argtype[0]);
    for(iarg=0; iarg<args->nargs; iarg++) {
        
        switch(__argtype[iarg]) {

#define _CASE(i, v_from, v_to, operation)       \
        case v_from:                            \
            args->argtype[i] = v_to;            \
            args->pre_size += SIZEOF_FAT(v_to); \
            operation;                          \
            break;  

        _CASE(iarg, PA_INT, FAT_INT,);
        _CASE(iarg, PA_INT|PA_FLAG_LONG, FAT_LONG_INT,);
        _CASE(iarg, PA_INT|PA_FLAG_SHORT, FAT_SHORT,);
        _CASE(iarg, PA_INT|PA_FLAG_LONG_LONG, FAT_LONG_LONG_INT,);

        _CASE(iarg, PA_CHAR, FAT_CHAR,);
        _CASE(iarg, PA_WCHAR, FAT_SHORT,);   //wchar_t -> short
        _CASE(iarg, PA_STRING, FAT_STRING, args->has_string = 1);   //只有当参数中存在 字符串时，
        _CASE(iarg, PA_POINTER, FAT_POINTER,);
        _CASE(iarg, PA_FLOAT, FAT_FLOAT,);
        _CASE(iarg, PA_DOUBLE, FAT_DOUBLE,);
        _CASE(iarg, PA_DOUBLE|PA_FLAG_LONG_DOUBLE, FAT_LONG_DOUBLE,);
        
#undef _CASE

        default:
            printf("\t%d unknown(%d).\n", iarg, __argtype[iarg]);
            assert(0 && "Not support wstring type.");
            break;
        }

        
    }
    
    return args->nargs;
}


