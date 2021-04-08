#include <string.h>
#include <assert.h>
#include "zmalloc.h"
#include "dict.h"
#include "sds.h"

static uint64_t dictSdsCaseHash(const void *key) {
    return dictGenCaseHashFunction((unsigned char*)key, sdslen((char*)key));
}
void dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

/* A case insensitive version used for the command lookup table and other
 * places where case insensitive non binary-safe comparison is needed. */
int dictSdsKeyCaseCompare(void *privdata, const void *key1,
        const void *key2)
{
    DICT_NOTUSED(privdata);

    return strcasecmp(key1, key2) == 0;
}

/* Command table. sds string -> command struct pointer. */
dictType commandTableDictType = {
    dictSdsCaseHash,            /* hash function */
    NULL,                       /* key dup */
    NULL,                       /* val dup */
    dictSdsKeyCaseCompare,      /* key compare */
    dictSdsDestructor,          /* key destructor */
    NULL,                       /* val destructor */
    NULL                        /* allow to expand */
};

#define NR_MOD  256

struct module {
    char name[64];
    unsigned long id;
} modules[NR_MOD] = {
    {"CUCP", },
    {"CUUP-1", },
    {"CUUP-2", },
    {"CUUP-3", },
    {"CUUP-4", },
    {"DU-1", },
    {"DU-2", },
    {"DU-3", },
    {"DU-4", },
};

void register_module(dict *d, char *name, unsigned long id) 
{
    dictAdd(d, name, (void*)id);
    dictEntry *entry = dictFind(d, name);
    printf("%4d# %-10s -> ID %4ld.\n", dictSize(d), name, (unsigned long)dictGetVal(entry));
}

unsigned long find_module_id(dict *d, char *name) 
{
    dictEntry *entry = dictFind(d, name);
    return (unsigned long)dictGetVal(entry);
}

int main(int argc, char **argv)
{
    int i;
    struct module *pmod;
    struct timeval start, end;
    
    dict *d1 = dictCreate(&commandTableDictType,NULL);
    dictExpand(d1, NR_MOD);
    
    for(i=0; i<NR_MOD; i++) {
        if(strlen(modules[i].name) <= 1)
            snprintf(modules[i].name, 64, "MOD%c%c%c-%d", 
                        'A'+randomULong()%26, 'A'+randomULong()%26, 'A'+randomULong()%26, i+1);
        
        modules[i].id = i+1;
        register_module(d1, modules[i].name, modules[i].id);
    }

    unsigned long find_times = 1000000;

    gettimeofday(&start, NULL);
    for(i=0; i<find_times; i++) {
        pmod = &modules[i%NR_MOD];
        if(find_module_id(d1, pmod->name) != pmod->id) {
            printf("find module id error, is %ld, need %ld\n", find_module_id(d1, pmod->name), pmod->id);
            assert(0 && "Wrong id");
        }
    }

    gettimeofday(&end, NULL);

    unsigned long usec = (end.tv_sec - start.tv_sec)*1000000
                                + (end.tv_usec - start.tv_usec);

    printf("Find %ld, total spend %ld us (%.4lf ms) (%.4lf s).\n",
            find_times, usec, usec/1000.0, usec/1000000.0);
    
    
    return 0;
}
