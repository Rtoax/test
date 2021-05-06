#include <stdio.h>

#define CONFIG_X86_5LEVEL

#define pgtable_l5_enabled() 1

#define PAGE_SIZE 4096
#define DEFAULT_MAP_WINDOW	((1UL << 47) - PAGE_SIZE)

#ifdef CONFIG_X86_5LEVEL
#define __VIRTUAL_MASK_SHIFT	(pgtable_l5_enabled() ? 56 : 47)
#else
#define __VIRTUAL_MASK_SHIFT	47
#endif

#define _AC(a,b) a##b

#define TASK_SIZE_MAX	((_AC(1,UL) << __VIRTUAL_MASK_SHIFT) - PAGE_SIZE)

int main() {
	printf("0x%016lx\n", TASK_SIZE_MAX);
}
