/**
 * https://gcc.gcc.gnu.narkive.com/WvwssETm/builtin-apply-args-gcc-4-1-1
 *	gcc-src/gcc/testsuite/gcc.dg/builtin-apply2.c
 */
#include <stdio.h>
#include <stdarg.h>

union func_union {
    void (*f1)();
    void (*f2)(char *fmt, ...);
};



void foo(char *name, int d, int e, int f, int g)
{
	printf("\nname = %s\n13 = %d\n72 = %d\n89 = %d\n5 = %d\n", name,d,e,f,g );
}
void foo2(char *fmt, ...)
{
	va_list va;

    va_start(va, fmt);

    vprintf(fmt, va);

    va_end(va);
}

void bar(char *name, ...)
{
	void **arg;
    void *rslt;
    
	arg = __builtin_apply_args();
    
	rslt = __builtin_apply(foo, arg, 24);
	rslt = __builtin_apply(foo2, arg, 24);
    
    __builtin_return (rslt);
}

int main(void)
{
	bar("eeee: %d %d %d %d\n", 13, 72, 89, 5);
	return 0;
}

