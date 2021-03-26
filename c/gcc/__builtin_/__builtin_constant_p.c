#include <stdio.h>

#define PREDEFINED_VAL 1

int main() {
	int i = 5;
	printf("__builtin_constant_p(i) is %d\n", __builtin_constant_p(i));
	printf("__builtin_constant_p(PREDEFINED_VAL) is %d\n", __builtin_constant_p(PREDEFINED_VAL));
	printf("__builtin_constant_p(100) is %d\n", __builtin_constant_p(100));

	return 0;
}
