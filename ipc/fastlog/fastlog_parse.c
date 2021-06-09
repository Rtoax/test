#define _GNU_SOURCE
#include <fastlog.h>
#include <regex.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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






static int mmap_fastlog_logfile(struct fastlog_file_mmap *mmap_file, char *filename, char *backupfilename, size_t size, 
                        int open_flags, int mmap_prot, int mmap_flags)
{
    assert(mmap_file && filename && "NULL pointer error");

    if(backupfilename) {
        if(access(filename, F_OK) == 0) {
            printf("File %s exist.\n", filename);
            rename(filename, backupfilename);
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

    mmap_file->mmapaddr = mmap(NULL, size, mmap_prot, mmap_flags, mmap_file->fd,0);
    if(mmap_file->mmapaddr == MAP_FAILED) {
        assert(0 && "mmap failed.");
    }


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

    int ret = munmap(mmap_file->mmapaddr, mmap_file->mmap_size);
    if(ret != 0) {
        printf("munmap %s failed.\n", mmap_file->filepath);
    }
    
    free(mmap_file->filepath);
    
    close(mmap_file->fd);
}
