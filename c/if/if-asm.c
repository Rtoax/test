#include <stdio.h>

int main() {
	int a = -1;
	if(a) {
		printf("if(%d) is ok.\n", a);
	} else {
		printf("if(%d) is not ok.\n", a);
	}
	if(!a) {
		printf("if(%d) is ok.\n", !a);
	} else {
		printf("if(%d) is not ok.\n", !a);
	}
}
