#define _GNU_SOURCE
#include <fastlog.h>
#include <regex.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fastlog_utils.h>


extern __thread struct StagingBuffer *stagingBuffer;

extern struct StagingBuffer *threadBuffers[1024];
extern fastlog_atomic64_t  stagingBufferId;


/**
 *  通知 FastLog 后台线程，日志发生，取决于宏`_FASTLOG_USE_EVENTFD`
 */
#if _FASTLOG_USE_EVENTFD
extern __thread int stagingBuffer_event_fd;
#endif



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




/* 通过 */
inline int 
__fastlog_print_buffer(int log_id, struct args_type *args, ...)
{
    int iarg;
    va_list va, va2;
    va_start(va, args);
    va_start(va2, args);
    
    struct arg_hdr *arghdr = NULL;
    char *args_data = NULL;
    unsigned int args_data_bytes = args->pre_size;

    //如果有字符串，需要求取字符串长度
    if(unlikely(args->has_string == 1)) {
//        printf("args->has_string = %d, pre_size = %d\n", args->has_string, args->pre_size);
        
        for(iarg=0; iarg<args->nargs; iarg++) {
            switch(args->argtype[iarg]) {

#define _CASE(fat_type, type)                       \
        case fat_type: {                            \
            type _unused _val = va_arg(va, type);           \
            break;                                  \
        }
#define _CASE_STRING(fat_type, type)                \
        case fat_type: {                            \
            type _unused _val = va_arg(va, type);           \
            /*printf("STR:%s\n", _val); */          \
            if(FAT_STRING == fat_type) {            \
                args_data_bytes += strlen(_val)+1;  \
            }                                       \
            break;                                  \
        }
            _CASE(FAT_INT8, int);
            _CASE(FAT_INT16, int);
            _CASE(FAT_INT32, int32_t);
            _CASE(FAT_INT64, int64_t);
            
            _CASE(FAT_UINT8, unsigned int);
            _CASE(FAT_UINT16, unsigned int);
            _CASE(FAT_UINT32, uint32_t);
            _CASE(FAT_UINT64, uint64_t);
//            
            _CASE(FAT_INT, int);
            _CASE(FAT_SHORT, int);
            _CASE(FAT_SHORT_INT, int);

            _CASE(FAT_LONG, long);
            _CASE(FAT_LONG_INT, long int);
            _CASE(FAT_LONG_LONG, long long);
            _CASE(FAT_LONG_LONG_INT, long long int);

            _CASE(FAT_CHAR, int);
            _CASE(FAT_UCHAR, unsigned int);
            
            _CASE_STRING(FAT_STRING, char *);

            _CASE(FAT_POINTER, void*);
            
            _CASE(FAT_FLOAT, double);
            _CASE(FAT_DOUBLE, double);
            _CASE(FAT_LONG_DOUBLE, long double);
            
            default:
                printf("\t%d unknown(%d).\n", iarg, args->argtype[iarg]);
                assert(0 && "Not support type." && __FILE__ && __LINE__);
                break;
#undef _CASE
#undef _CASE_STRING
            }
        }
    }
    

try_reserve_again:
    /**
     *  stagingBuffer 在`ensureStagingBufferAllocated`确保不为空
     */
    arghdr = (struct arg_hdr *)reserveProducerSpace(stagingBuffer, args_data_bytes + sizeof(struct arg_hdr));

    if(!arghdr) {
        goto try_reserve_again;
    }
    args_data = arghdr->log_args_buff;
    
    arghdr->log_id = log_id;

    for(iarg=0; iarg<args->nargs; iarg++) {

//        printf("%d->%d(%s)\n", iarg, args->argtype[iarg], FASTLOG_FAT_TYPE2SIZENAME[args->argtype[iarg]].name);
//        sleep(1);
        switch(args->argtype[iarg]) {

#define _CASE(fat_type, type)                       \
        case fat_type: {                            \
            type _val = (type)va_arg(va2, type);    \
            memcpy(args_data, &_val, sizeof(type)); \
            args_data += sizeof(type);              \
            break;                                  \
        }
#define _CASE_STRING(fat_type, type)                \
        case fat_type: {                            \
            type _val = (type)va_arg(va2, type);    \
            int len = strlen(_val)+1;               \
            memcpy(args_data, _val, len);           \
            args_data += len;                       \
            break;                                  \
        }
#define _CASE_POINTER(fat_type, type)                   \
        case fat_type: {                                \
            type _val = (type)va_arg(va2, type);        \
            unsigned long __v = (unsigned long)_val;    \
            memcpy(args_data, &__v, sizeof(type));      \
            args_data += sizeof(type);                  \
            break;                                      \
        }

        _CASE(FAT_INT8, int);
        _CASE(FAT_INT16, int);
        _CASE(FAT_INT32, int32_t);
        _CASE(FAT_INT64, int64_t);
        
        _CASE(FAT_UINT8, unsigned int);
        _CASE(FAT_UINT16, unsigned int);
        _CASE(FAT_UINT32, uint32_t);
        _CASE(FAT_UINT64, uint64_t);
        
        _CASE(FAT_INT, int);
        _CASE(FAT_SHORT, int);
        _CASE(FAT_SHORT_INT, int);

        _CASE(FAT_LONG, long);
        _CASE(FAT_LONG_INT, long int);
        _CASE(FAT_LONG_LONG, long long);
        _CASE(FAT_LONG_LONG_INT, long long int);

        _CASE(FAT_CHAR, int);
        _CASE(FAT_UCHAR, unsigned int);
        
        _CASE_STRING(FAT_STRING, char *);

        _CASE_POINTER(FAT_POINTER, void*);
        
        _CASE(FAT_FLOAT, double);
        _CASE(FAT_DOUBLE, double);
        _CASE(FAT_LONG_DOUBLE, long double);
        
        default:
            printf("\t%d unknown(%d).\n", iarg, args->argtype[iarg]);
            assert(0 && "Not support type." && __FILE__ && __LINE__);
            break;
#undef _CASE
#undef _CASE_STRING
#undef _CASE_POINTER
        }
    
    }

    va_end(va);
    va_end(va2);

    arghdr->log_args_size = args_data_bytes;
    arghdr->log_rdtsc = __fastlog_rdtsc();

    //确认消费
    finishReservation(stagingBuffer, args_data_bytes + sizeof(struct arg_hdr));

#if _FASTLOG_USE_EVENTFD
    //fprintf(stderr, "eventfd_write stagingBuffer_event_fd = %d\n", stagingBuffer_event_fd);
    eventfd_write(stagingBuffer_event_fd, 1);
#endif

//    printf("args_data_bytes = %d\n", args_data_bytes);
    return args_data_bytes;
}





static int mmap_fastlog_logfile(struct fastlog_file_mmap *mmap_file, char *filename, char *backupfilename, size_t size, 
                        int open_flags, int mmap_prot, int mmap_flags)
{
    assert(mmap_file && filename && "NULL pointer error");

    if(access(filename, F_OK) == 0) {
        //printf("File %s exist.\n", filename);
        /* 如果备份文件名设定 */
        if(backupfilename) {
            rename(filename, backupfilename);
        }
    } else {
        if(O_RDONLY == open_flags) {    //文件不存在，但是只读打开
            printf("File %s not exist.\n", filename);
            return -1;
        }
    }

    mmap_file->filepath = strdup(filename);
    mmap_file->mmap_size = size;

    //文件映射
    //截断打开/创建文件
    mmap_file->fd = open(filename, open_flags, 0644);
    if(mmap_file->fd == -1) {
        assert(0 && "Open failed.");
    }

    if(size) {
        if((ftruncate(mmap_file->fd, size)) == -1) {
            assert(0 && "ftruncate failed.");
        }
    } else {
        struct stat stat_buf;
        stat(filename, &stat_buf);
        size = mmap_file->mmap_size = stat_buf.st_size;
    }
//    printf("mmap_file->mmap_size = %d\n", mmap_file->mmap_size);

    mmap_file->mmapaddr = mmap(NULL, size, mmap_prot, mmap_flags, mmap_file->fd, 0);
    if(mmap_file->mmapaddr == MAP_FAILED) {
        assert(0 && "mmap failed.");
    }

    /**
     *  当不是读方映射的文件，需要清零，原因在于，当用户指定了允许输出的最大文件大小
     *  和最大日志文件个数后，如果超出使用限制，将覆盖前面已经生成的文件，以防止磁盘
     *  写满。
     *  若不清零，写者可能不会写到文件大小的结尾处，造成解析过程出错，如下图：
     *
     *  第一次写者打开
     *  ----------------------------------------------------
     *
     *  第一次写者写满
     *  ##################################################-- (放不下一条日志时，空闲)
     *
     *  第二次写者打开(不清零)
     *  ##################################################--
     *  写入新的数据
     *  ***********************************************###-- (放不下一条日志时，空闲)
     *  此时读者打开
     *  ***********************************************###-- (当解析完`*`后，`#`为错误信息)
     *
     *  2021年6月23日 荣涛
     */
    if(O_RDONLY != open_flags) {
        memset(mmap_file->mmapaddr, 0, size);
    }
    
    mmap_file->status = FILE_MMAP_MMAPED;

    return 0;
}

/* 映射元数据文件-读写 */
int mmap_fastlog_logfile_write(struct fastlog_file_mmap *mmap_file, char *filename, char *backupfilename, size_t size)
{
    return mmap_fastlog_logfile(mmap_file, filename, backupfilename, size, 
                            O_RDWR|O_CREAT|O_TRUNC, PROT_WRITE|PROT_READ, MAP_SHARED);
}
void msync_fastlog_logfile_write(struct fastlog_file_mmap *mmap_file)
{
    msync(mmap_file->mmapaddr, mmap_file->mmap_size, MS_ASYNC);
}

/* 映射元数据文件-只读 */
int mmap_fastlog_logfile_read(struct fastlog_file_mmap *mmap_file, char *filename)
{
    return mmap_fastlog_logfile(mmap_file, filename, NULL, 0, 
                            O_RDONLY, PROT_READ, MAP_SHARED);
}


int unmmap_fastlog_logfile(struct fastlog_file_mmap *mmap_file)
{
    assert(mmap_file && "NULL pointer error");

    if(mmap_file->status == FILE_MMAP_NULL) {
        return 0;
    }

    int ret = munmap(mmap_file->mmapaddr, mmap_file->mmap_size);
    if(ret != 0) {
        printf("munmap %s failed.\n", mmap_file->filepath);
    }
    
    mmap_file->status = FILE_MMAP_NULL;
    
    free(mmap_file->filepath);
    
    close(mmap_file->fd);

    return 0;
}
