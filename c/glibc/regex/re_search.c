#define __USE_GNU
#include <regex.h>
#include <stdio.h>

//int re_search(struct re_pattern_buffer *__buffer, const char *__string,
//		      int __length, int __start, int __range,
//		      struct re_registers *__regs);

int main()
{
    re_search(NULL, NULL, 0, 0, 0, NULL);
}

