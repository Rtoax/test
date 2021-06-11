#include "varargs2.h"

extern printf();

main()
{
	va_stack(stack, 10);	/* declare vector which holds up to 10 args */

	va_stack_init(stack);

	va_push(stack, "%d %f %s\n", char *);
	va_push(stack, 12, int);
	va_push(stack, 3.14, double);
	va_push(stack, "Hello, world!", char *);

	va_call(printf, stack);
}
