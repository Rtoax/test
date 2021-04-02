#include <stdio.h>

#define CONFIG_PHYSICAL_ALIGN 0x200000
#define CONFIG_PHYSICAL_START 0x1000000 /* (16 MB) */

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~ __round_mask(x, y))

#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a)		__ALIGN_KERNEL((x), (a))

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE-1))


#define PGDIR_SHIFT		39
#define PUD_SHIFT	30
#define PMD_SHIFT	21

#define PMD_SIZE	(1UL << PMD_SHIFT)
#define PMD_MASK	(~(PMD_SIZE - 1))
#define PUD_SIZE	(1UL << PUD_SHIFT)
#define PUD_MASK	(~(PUD_SIZE - 1))

#define PTRS_PER_PGD		512
#define PTRS_PER_PUD	512
#define PTRS_PER_PMD	512
#define PTRS_PER_PTE	512

#define VSYSCALL_ADDR (-10UL << 20)

#define FIXADDR_TOP	(round_up(VSYSCALL_ADDR + PAGE_SIZE, 1<<PMD_SHIFT) - \
					 PAGE_SIZE)

#define FIXADDR_SIZE	(__end_of_permanent_fixed_addresses << PAGE_SHIFT)
#define FIXADDR_START		(FIXADDR_TOP - FIXADDR_SIZE)

#define __START_KERNEL_map	0xffffffff80000000UL/* kernel 起始地址 */
#define __START_KERNEL		(__START_KERNEL_map + __PHYSICAL_START) /*  */

#define __PHYSICAL_START /*  */	ALIGN(CONFIG_PHYSICAL_START, \
						      CONFIG_PHYSICAL_ALIGN)

#define KERNEL_IMAGE_SIZE	(1024 * 1024 * 1024)

#define MODULES_VADDR		(__START_KERNEL_map + KERNEL_IMAGE_SIZE)


#define NR_FIX_BTMAPS		64
#define FIX_BTMAPS_SLOTS	8
#define TOTAL_FIX_BTMAPS	(NR_FIX_BTMAPS * FIX_BTMAPS_SLOTS)


#define MAX_IO_APICS 128
#define MAX_LOCAL_APIC 32768

#define __fix_to_virt(x)	(FIXADDR_TOP - ((x) << PAGE_SHIFT))
#define __virt_to_fix(x)	((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)

enum fixed_addresses {  /* 每个固定映射的地址都由一个整数索引表示 */
	VSYSCALL_PAGE = (FIXADDR_TOP - VSYSCALL_ADDR) >> PAGE_SHIFT,    /* =  `511`*/
	FIX_DBGP_BASE,
	FIX_EARLYCON_MEM_BASE,
	FIX_OHCI1394_BASE,
	FIX_APIC_BASE,	/* local (CPU) APIC) -- required for SMP or not */
	FIX_IO_APIC_BASE_0,
	FIX_IO_APIC_BASE_END = FIX_IO_APIC_BASE_0 + MAX_IO_APICS - 1,
	FIX_PARAVIRT_BOOTMAP,
	FIX_APEI_GHES_IRQ,
	FIX_APEI_GHES_NMI,

	__end_of_permanent_fixed_addresses,/*  */

	FIX_BTMAP_END =
	 (__end_of_permanent_fixed_addresses ^
	  (__end_of_permanent_fixed_addresses + TOTAL_FIX_BTMAPS - 1)) &
	 -PTRS_PER_PTE
	 ? __end_of_permanent_fixed_addresses + TOTAL_FIX_BTMAPS -
	   (__end_of_permanent_fixed_addresses & (TOTAL_FIX_BTMAPS - 1))
	 : __end_of_permanent_fixed_addresses,
	FIX_BTMAP_BEGIN = FIX_BTMAP_END + TOTAL_FIX_BTMAPS - 1,
	FIX_TBOOT_BASE,
	__end_of_fixed_addresses
};

static __always_inline unsigned long fix_to_virt(const unsigned int idx)
{
	//BUILD_BUG_ON(idx >= __end_of_fixed_addresses);
	return __fix_to_virt(idx);
}

static inline unsigned long virt_to_fix(const unsigned long vaddr)
{
	//BUG_ON(vaddr >= FIXADDR_TOP || vaddr < FIXADDR_START);
	return __virt_to_fix(vaddr);
}


#define print_mm(v) printf("[FIX] %016lx : %s\n", v, #v);

static void print_fixmap() {
	print_mm(__PHYSICAL_START);
	print_mm(PAGE_MASK);
    print_mm(FIXADDR_SIZE);
    
	print_mm(0xffffffffffffffff);
	print_mm(FIXADDR_TOP);
    print_mm(fix_to_virt(VSYSCALL_PAGE));
    print_mm(fix_to_virt(FIX_DBGP_BASE));
    print_mm(fix_to_virt(FIX_APIC_BASE));
    print_mm(fix_to_virt(FIX_IO_APIC_BASE_0));
    print_mm(fix_to_virt(FIX_PARAVIRT_BOOTMAP));
    print_mm(fix_to_virt(FIX_APEI_GHES_IRQ));
    print_mm(fix_to_virt(FIX_APEI_GHES_NMI));
	print_mm(FIXADDR_START);
    
    print_mm(fix_to_virt(__end_of_permanent_fixed_addresses));
    print_mm(fix_to_virt(FIX_BTMAP_END));
    print_mm(fix_to_virt(FIX_BTMAP_BEGIN));
    print_mm(fix_to_virt(FIX_TBOOT_BASE));
    print_mm(fix_to_virt(__end_of_fixed_addresses));
	print_mm(MODULES_VADDR);
	print_mm(__START_KERNEL);
	print_mm(__START_KERNEL_map);

}


#define __print_val(prefix, v) 

#define print_iomap(v, p) printf("[ioremap] %016lx : %016lx\n", v, p);

void early_ioremap_setup(void)
{
    int i;
    for (i = 0; i < FIX_BTMAPS_SLOTS; i++) {
        unsigned long phy = (FIX_BTMAP_BEGIN - NR_FIX_BTMAPS*i);
        unsigned long virt = __fix_to_virt(phy);
        print_iomap(virt, phy);
    }
            
}
void early_ioremap_init(void){
    early_ioremap_setup();
    unsigned long phy = FIX_BTMAP_BEGIN;
    unsigned long virt = fix_to_virt(FIX_BTMAP_BEGIN);

    print_iomap(virt, phy);
}

static void print_ioremap() {
    early_ioremap_init();
}

int main() {
    print_fixmap();

    print_ioremap();
}
