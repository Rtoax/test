#include <stdio.h>

#define O_ACCMODE	00000003
#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002

#define ACC_MODE(x) ("\004\002\006\006"[(x)&O_ACCMODE]) 

int main() {

	printf("0x%x\n", ACC_MODE(O_RDONLY));
	printf("0x%x\n", ACC_MODE(O_WRONLY));
	printf("0x%x\n", ACC_MODE(O_RDWR));
}
