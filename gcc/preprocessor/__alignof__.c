#include <stdio.h>

struct a {
	int a;
	long b;
};

struct b {
	int a;
	long b;
}__attribute__((packed));

int main() {
	printf("%d\n", __alignof__(struct a));
	printf("%d\n", __alignof__(struct b));
}
