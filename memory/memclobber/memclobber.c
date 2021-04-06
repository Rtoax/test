#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include <memlist.h>
#include <memtools.h>
#include <memclobber.h>

/* canary magic 数，用于特殊的标记 */
#define CANARY_MAGIC    0xcbaaabcd



/* 调试信息 */
struct mdebug_info {
    unsigned long idx; 
    size_t used_memory;         //使用的内存总数
    struct list_head chunks;
    pthread_rwlock_t chunk_list_lock; //PTHREAD_RWLOCK_INITIALIZER
};

struct mchunk {
    unsigned long  size; 
    struct list_head list;
    const char __pad[16];
    unsigned long *magic;
};


/* 总内存大小 */
static size_t total_used_memory = 0;

/* 最大支持256个内存块 */
static struct mdebug_info *debug_info;



static pthread_t memclobber_threadid;

static void *memclobber_routine(void*arg){
    struct hpmem_chunk *chunk, *iter_next;
    
    while(1) {
//        pthread_rwlock_rdlock(&chunk_list_lock); 
//        list_for_each_entry_safe(chunk, iter_next, &chunk_list_head, list) {
//            hpmem_chunk_is_ok_abort(chunk);
//        }
//        pthread_rwlock_unlock(&chunk_list_lock); 
        usleep(10000);
    }
    
}

extern void __memhook_init();


static  __attribute__((constructor(101))) void __memclobber_init() {
    __memhook_init();
    pthread_create(&memclobber_threadid, NULL, memclobber_routine, NULL);
    
}

static int memclobber_abort(struct mchunk *chunk)
{
#if 0
    if(unlikely(HPMEM_CHUNK_MAGIC_HEAD(chunk) != HPMEM_MAGIC)) {
        fprintf(stderr, "\033[1;5;31m[ID:%ld, %d Bytes] %s, addr %p\033[m\n", 
                                                          HPMEM_CHUNK_MODULE_ID(chunk), 
                                                          HPMEM_CHUNK_SIZE(chunk), 
                                                          hpmem_strerror[HPMEM_EHEAD].e_string,
                                                          chunk->addr);
        goto mem_dump_error;
    }
    if(unlikely(HPMEM_CHUNK_MAGIC_TAIL(chunk) != HPMEM_MAGIC)) {
        fprintf(stderr, "\033[1;5;31m[ID:%ld, %d Bytes] %s, addr %p\033[m\n", 
                                                          HPMEM_CHUNK_MODULE_ID(chunk), 
                                                          HPMEM_CHUNK_SIZE(chunk), 
                                                          hpmem_strerror[HPMEM_ETAIL].e_string,
                                                          chunk->addr);
        goto mem_dump_error;
    }
    if(unlikely(HPMEM_CHUNK_MODULE_ID(chunk) <= 0)) {
        fprintf(stderr, "\033[1;5;31m[%d Bytes] %s, addr %p\033[m\n", 
                                                          HPMEM_CHUNK_SIZE(chunk), 
                                                          hpmem_strerror[HPMEM_EPARAM].e_string,
                                                          chunk->addr);
        goto mem_dump_error;
    }
    if(unlikely(HPMEM_CHUNK_SIZE(chunk) <= 0)) {
        fprintf(stderr, "\033[1;5;31m[ID:%ld] %s, addr %p\033[m\n", 
                                                          HPMEM_CHUNK_MODULE_ID(chunk),
                                                          hpmem_strerror[HPMEM_EPARAM].e_string,
                                                          chunk->addr);
        goto mem_dump_error;
    }
    return 1;
    
    
mem_dump_error:
#endif
    fprintf(stderr, "hpmalloc: memory used ERROR.\n");
    fflush(stderr);
    abort();
}




