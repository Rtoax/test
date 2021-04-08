#include "zmalloc.h"
#include "dict.h"
#include "sds.h"

static uint64_t dictSdsHash(const void *key) {
    return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}
static int dictSdsKeyCompare(void *privdata, const void *key1,
        const void *key2)
{
    int l1,l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}
void dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}


/* Db->dict, keys are sds strings, vals are Redis objects. */
dictType dbDictType = {
    dictSdsHash,                /* hash function */
    NULL,                       /* key dup */
    NULL,                       /* val dup */
    dictSdsKeyCompare,          /* key compare */
    dictSdsDestructor,          /* key destructor */
    NULL,                       /* val destructor */
    NULL,                       /* allow to expand */
};

#define NR_MOD  256

struct module {
    char name[64];
    unsigned long id;
} modules[NR_MOD] = {{0}, 0};

void register_module(dict *d, char *name, unsigned long id) 
{
    dictAdd(d, name, (void*)id);
    dictEntry *entry = dictFind(d, name);
    printf("%4d# %10s -> ID %4ld.\n", dictSize(d), name, (unsigned long)entry->v.u64);
}

int main(int argc, char **argv)
{
    int i;
    dict *d1 = dictCreate(&dbDictType,NULL);

    for(i=0; i<NR_MOD; i++) {
        snprintf(modules[i].name, 64, "MODULE%d", i+1);
        modules[i].id = i+10;
        register_module(d1, modules[i].name, modules[i].id);
    }

    //TODO：求出来的hash值相同 TODO
    
    
    return 0;
}
