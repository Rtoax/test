#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct test {
	int id;
	unsigned long ul;
	char name[13];
	char data[0];
};


int main()
{
	struct test T[10];

	struct test *t2 = &T[2];

	printf("idx = %d\n", t2 - T);
	printf("idx = %d\n", (unsigned long)(t2 - T));
	printf("%d\n", (unsigned long)t2 - (unsigned long)T);

	return 0;
}
