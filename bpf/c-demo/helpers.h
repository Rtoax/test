/* Misc helper macros. */
#define __section(x) __attribute__((section(x), used))
#define offsetof(x, y) __builtin_offsetof(x, y)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/* Object pinning settings */
#define PIN_NONE       0
#define PIN_OBJECT_NS  1
#define PIN_GLOBAL_NS  2

/* ELF map definition */
struct bpf_elf_map {
   __u32 type;
   __u32 size_key;
   __u32 size_value;
   __u32 max_elem;
   __u32 flags;
   __u32 id;
   __u32 pinning;
   __u32 inner_id;
   __u32 inner_idx;
};

/* Some used BPF function calls. */
static int (*bpf_skb_store_bytes)(void *ctx, int off, void *from,
                                 int len, int flags) =
     (void *) BPF_FUNC_skb_store_bytes;
static int (*bpf_l4_csum_replace)(void *ctx, int off, int from,
                                 int to, int flags) =
     (void *) BPF_FUNC_l4_csum_replace;
static void *(*bpf_map_lookup_elem)(void *map, void *key) =
     (void *) BPF_FUNC_map_lookup_elem;

/* Some used BPF intrinsics. */
unsigned long long load_byte(void *skb, unsigned long long off)
   asm ("llvm.bpf.load.byte");
unsigned long long load_half(void *skb, unsigned long long off)
   asm ("llvm.bpf.load.half");

