/**
 * https://gcc.gcc.gnu.narkive.com/WvwssETm/builtin-apply-args-gcc-4-1-1
 *	gcc-src/gcc/testsuite/gcc.dg/builtin-apply2.c
 */
#include <stdio.h>
#include <stdarg.h>

#define INTEGER_ARG 5

void * __builtin_apply (void (*function)(), void *arguments, size_t size);
void ** __builtin_apply_args();
void __builtin_return (void *result);
__builtin_va_arg_pack ();
size_t __builtin_va_arg_pack_len ();



void foo(char *name, int d, int e, int f, int g)
{
	printf("\nname = %s\n13 = %d\n72 = %d\n89 = %d\n5 = %d\n", name,d,e,f,g );
}

void bar(char *name, ...)
{
	void **arg;
	arg = __builtin_apply_args();
    //void * __builtin_apply (void (*function)(), void *arguments, size_t size);
	__builtin_apply(foo, arg, 24);
}

int main(void)
{
	bar("eeee", 13, 72, 89, INTEGER_ARG);
	return 0;
}
