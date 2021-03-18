#include <stdio.h>
#include <string.h>

/**
 *	CPUID操作码是一个面向x86架构的处理器补充指令，它的名称派生自CPU识别，
 *	作用是允许软件发现处理器的详细信息。它由英特尔在1993年引入奔腾和SL增强486处理器。
 */
static void cpuid( unsigned int n,
                    unsigned int* a, unsigned int* b,
                    unsigned int* c, unsigned int* d )
{
	__asm__ __volatile__ (
		"cpuid"
		: "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)      /* output */
		: "0" (n)         /* input */
	);
}

static void vendorString()
{
	char vstr[13];
	unsigned int a, b, c, d;
	cpuid(0, &a, &b, &c, &d);
	memcpy(&vstr[0], &b, 4);
	memcpy(&vstr[4], &d, 4);
	memcpy(&vstr[8], &c, 4);
	vstr[12] = 0;
	printf("vender = %s\n", vstr);
}

int main() {

	vendorString();

	return 0;
}
