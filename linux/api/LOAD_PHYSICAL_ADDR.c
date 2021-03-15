#include <stdio.h>


#define CONFIG_PHYSICAL_ALIGN 0x200000
#define CONFIG_PHYSICAL_START 0x1000000

/* Physical address where kernel should be loaded. */
#define LOAD_PHYSICAL_ADDR ((CONFIG_PHYSICAL_START/* 0x1000000 */ \
							+ (CONFIG_PHYSICAL_ALIGN/* 0x200000 */ - 1)/* 0x1fffff */) \
						& ~(CONFIG_PHYSICAL_ALIGN/* 0x200000 */ - 1)/*  */)

int main() {
	printf("LOAD_PHYSICAL_ADDR = %lx\n", LOAD_PHYSICAL_ADDR);
}
