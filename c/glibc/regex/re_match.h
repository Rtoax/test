#define __USE_GNU
#include <regex.h>

int re_match(struct re_pattern_buffer *__buffer, const char *__string,
		     int __length, int __start, struct re_registers *__regs);

