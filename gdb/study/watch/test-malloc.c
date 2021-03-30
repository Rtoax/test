#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

struct A {
	int a;
	int b;
	int c;
#define A_INIT {1,2,3}
};

#if 0
struct A A[] = {
A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,
A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,
A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,
A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,A_INIT,
};
void init_A(){}
void delete_A(){}
#else
struct A *A = NULL;
void init_A() {
	int i;
	A = malloc(sizeof(struct A) * 32);
	for(i=0;i<32;i+=1) {
		A[i].a = A[i].b = A[i].c = i;
	}
}
void delete_A() {free(A);}
#endif

struct A* demo7(struct A* _a)
{
	int i;
	struct A *a = _a?_a:&A[8];
    for(i=0;i<16;i+=1) {
		a[i].a = a[i].b = a[i].c = i;
    }
    return a;
}
struct A* demo6(struct A* a){return demo7(a);}
struct A* demo5(struct A* a){return demo6(a);}
struct A* demo4(struct A* a){return demo5(a);}
struct A* demo3(struct A* a){return demo4(a);}
struct A* demo2(struct A* a){return demo3(a);}
struct A* demo1(struct A* a){return demo2(a);}

int main(int argc, char *argv[])
{
	init_A();

	struct A* a = demo1(NULL);

	a[0].a = a[2].b = a[4].c = 1024;

	(void*)demo1(a);

	delete_A();
}
