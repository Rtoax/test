#include <stdio.h>
#include <stdint.h>
#include <cpuid.h>

//test/gcc/x86_64-linux-gnu/4.8.5/include/cpuid.h

void my_get_cpuid()
{
	uint32_t __attribute__ ((unused)) eax, ebx, ecx, edx;
	if (__get_cpuid (1, &eax, &ebx, &ecx, &edx) == 0) {
		printf("call __get_cpuid error.\n");
	} else {
		printf("%#08x %#08x %#08x %#08x \n", eax, ebx, ecx, edx);
	}
}

int main()
{
	my_get_cpuid();
}

