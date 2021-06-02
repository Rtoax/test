#include <stdio.h>

int main()
{
#ifdef __x86_64__
	//gcc __x84_64__.c
	printf("__x86_64__\n");
#elif defined __i386__
	//gcc __x84_64__.c -m32
	printf("__i386__\n");
#endif
}
