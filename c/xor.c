#include <stdio.h>

int main()
{
	unsigned long flags = 0x12345678abcd0000;
	printf("%#lx\n", flags);

	flags ^= 0x00000000ffffffff;
	printf("%#lx\n", flags);
}
