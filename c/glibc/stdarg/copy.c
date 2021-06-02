#include <stdarg.h>
#include <stdio.h>

int print_fmt_copy(const char *fmt, ...)
{
	int ret = 0;
	va_list va;
	va_start(va, fmt);
	
	int i = va_arg(va, int);
	char *s = (char*)va_arg(va, char*);
	long l = va_arg(va, long);
	printf("%d %s %ld\n", i, s, l);

	va_end(va);
	return ret;
}


int main()
{
	print_fmt_copy("%d %s %ld\n", 1, "hello", 2L);	
}
