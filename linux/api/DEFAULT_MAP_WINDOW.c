#include <stdio.h>

#define PAGE_SIZE 4096
#define DEFAULT_MAP_WINDOW	((1UL << 47) - PAGE_SIZE)

int main() {
	printf("0x%0lx\n", DEFAULT_MAP_WINDOW);
}
