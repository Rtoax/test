#include <stdio.h>

int func() {
	__asm__("int3");
}

int main() {
    int i;
    while (i < 6){
	    printf("i equal to: %d\n", i);
	    __asm__("int3");
		++i;
		func();
    }
}
