#include <fastlog_decode.h>

/**
 *  映射的文件
 */
static struct fastlog_file_mmap ro_metadata_mmap, ro_logdata_mmap;

static struct fastlog_file_header *metadata_header = NULL;
static struct fastlog_file_header *logdata_header = NULL;




int release_file(struct fastlog_file_mmap *mapfile)
{
    return unmmap_fastlog_logfile(mapfile);
}

/* 以只读方式映射一个文件 */
static int load_file(struct fastlog_file_mmap *mapfile, const char *file, 
                        struct fastlog_file_header **hdr, unsigned int magic)
{
    int ret;
    
    //元数据文件映射
    ret = mmap_fastlog_logfile_read(mapfile, file);
    if(ret) {
        return -1;
    }
    *hdr = mapfile->mmapaddr;

    if((*hdr)->magic != magic) {
        fprintf(stderr, "%s is not fastlog metadata file.\n", file);
        release_file(mapfile);
        return -1;
    }

    return 0;
}
                        
struct fastlog_file_mmap *meta_mmapfile()
{
    return &ro_metadata_mmap;
}

struct fastlog_file_mmap *log_mmapfile()
{
    return &ro_logdata_mmap;
}

struct fastlog_file_header *meta_hdr()
{
    return metadata_header;
}
struct fastlog_file_header *log_hdr()
{
    return logdata_header;
}


int release_metadata_file()
{
    return release_file(&ro_metadata_mmap);
}
int release_logdata_file()
{
    return release_file(&ro_logdata_mmap);
}

int load_metadata_file(const char *mmapfile_name)
{
    return load_file(&ro_metadata_mmap, mmapfile_name, &metadata_header, FATSLOG_METADATA_HEADER_MAGIC_NUMBER);
}

int load_logdata_file(const char *mmapfile_name)
{
    return load_file(&ro_logdata_mmap, mmapfile_name, &logdata_header, FATSLOG_LOG_HEADER_MAGIC_NUMBER);
}

int match_metadata_and_logdata()
{
    uint64_t metadata_cycles_per_sec = metadata_header->cycles_per_sec;
    uint64_t metadata_start_rdtsc = metadata_header->start_rdtsc;
    uint64_t logdata_cycles_per_sec = logdata_header->cycles_per_sec;
    uint64_t logdata_start_rdtsc = logdata_header->start_rdtsc;

    /* 在写入时，这两个应该数值完全相等，所以这里进行检测 */
    return ((metadata_cycles_per_sec == logdata_cycles_per_sec) 
        && (metadata_start_rdtsc == logdata_start_rdtsc));
}

