#include <stdint.h>
#include <asm/types.h>

#include <linux/bpf.h>
#include <linux/pkt_sched.h>

#include "helpers.h"

struct tuple {
        long packets;
        long bytes;
};

#define BPF_MAP_ID_STATS        1 /* agent's map identifier */
#define BPF_MAX_MARK            256

struct bpf_elf_map __section("maps") map_stats = {
        .type           =       BPF_MAP_TYPE_ARRAY,
        .id             =       BPF_MAP_ID_STATS,
        .size_key       =       sizeof(uint32_t),
        .size_value     =       sizeof(struct tuple),
        .max_elem       =       BPF_MAX_MARK,
        .pinning        =       PIN_GLOBAL_NS,
};

static inline void cls_update_stats(const struct __sk_buff *skb,
                                    uint32_t mark)
{
        struct tuple *tu;

        tu = bpf_map_lookup_elem(&map_stats, &mark);
        if (likely(tu)) {
                __sync_fetch_and_add(&tu->packets, 1);
                __sync_fetch_and_add(&tu->bytes, skb->len);
        }
}

__section("cls") int cls_main(struct __sk_buff *skb)
{
        uint32_t mark = skb->mark;

        if (unlikely(mark >= BPF_MAX_MARK))
                return 0;

        cls_update_stats(skb, mark);

        return TC_H_MAKE(TC_H_ROOT, mark);
}

char __license[] __section("license") = "GPL";

