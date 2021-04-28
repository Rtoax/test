#include <stdio.h>

#ifdef NOLIKELY
#define likely(x) x
#define unlikely(x) x
#else
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

int main() {
	int a = 0;
	if(unlikely(a)) {
		printf("a > 0\n");
	} else {
		printf("a <= 0\n");
	}
}
