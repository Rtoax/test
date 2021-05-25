#include <stdio.h>
#include <stdarg.h>

const char *_global_fmt = NULL;
const void **p_fmt = NULL;

int print(const char *fmt, ...)
{
	_global_fmt = fmt;
	p_fmt = (void**)&fmt;

	va_list va;
	va_start(va, fmt);
	int ret = printf(fmt, va);
	va_end(va);
	return ret;
}

int main()
{
	print("print: %d\n", 1);
	sleep(1);
	printf("%s(%p)\n", _global_fmt, p_fmt);

	system("more /proc/$(pidof a.out)/maps");
}
