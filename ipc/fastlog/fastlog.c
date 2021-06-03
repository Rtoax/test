/**
 *  FastLog 低时延 LOG日志
 *
 *  代码日志
 *  2021年5月21日    创建源码文件
 *  2021年6月2日 
 *
 */

#include <fastlog.h>


#define __ro_after_init  


static uint64_t __ro_after_init __cycles_per_sec = 0;


static void
__fastlog_init_cycles() 
{
    if (__cycles_per_sec != 0)
        return;

    struct timeval startTime, stopTime;
    uint64_t startCycles, stopCycles, micros;
    double oldCycles;

    oldCycles = 0;
    while (1) {
        if (gettimeofday(&startTime, NULL) != 0) {
            fprintf(stderr, "Cycles::init couldn't read clock: %s", strerror(errno));
            assert(0);
        }
        startCycles = __fastlog_rdtsc();
        while (1) {
            if (gettimeofday(&stopTime, NULL) != 0) {
                fprintf(stderr, "Cycles::init couldn't read clock: %s", strerror(errno));
                assert(0);
            }
            stopCycles = __fastlog_rdtsc();
            micros = (stopTime.tv_usec - startTime.tv_usec) +
                    (stopTime.tv_sec - startTime.tv_sec)*1000000;
            if (micros > 10000) {
                __cycles_per_sec = (double)(stopCycles - startCycles);
                __cycles_per_sec = 1000000.0*__cycles_per_sec/(double)(micros);
                break;
            }
        }
        double delta = __cycles_per_sec/100000.0;
        if ((oldCycles > (__cycles_per_sec - delta)) &&
                (oldCycles < (__cycles_per_sec + delta))) {
            return;
        }
        oldCycles = __cycles_per_sec;
    }

    printf("__cycles_per_sec = %ld\n", __cycles_per_sec);
}


int fastlog_init()
{
    __fastlog_init_cycles();
}


int __fastlog_get_unused_id(int level, const char *name, char *file, char *func, int line, const char *format)
{
//    printf("get unused log id.\n");
    // TODO
    // 1. 生成 logid;
    // 2. 组件 logid 对应的元数据(数据结构)
    // 3. 保存元数据

    return 1; 
}



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

