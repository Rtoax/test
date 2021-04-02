#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdarg.h>

int my_log(int loglevel, const char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	char msg[200] = {0};

	vsnprintf(msg, 200, fmt, arg);
	printf(msg);

	va_end(arg);
}

int main() {
	char *s1 = malloc(64);
	uint16_t id = 19;

	my_log(1, "hello %ld world.\n", (unsigned long)id);

}
