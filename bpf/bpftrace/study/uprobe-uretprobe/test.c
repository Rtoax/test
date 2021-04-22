#include <stdio.h>

int func(int a) {
	return a + 1;
}

int main() {
	int a = 1;
	while(1) {
		a = func(a);
		sleep(1);
	}
}
