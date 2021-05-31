/**
 *  va_list 不能识别出 是否在编译阶段为常量函数
 *  荣涛 2021年5月31日
 */
#include <stdarg.h>
#include <stdio.h>

#define IS_CONST(i) __builtin_constant_p(i)

#define print(fmt, ...) \
	fprintf(stderr, "[rongtao]"__FILE__ fmt, ##__VA_ARGS__)


struct A {
#define MAGIC	0xff11
	int magic;
	int id;
};

void va_struct(int n, ...)
{
	va_list va;
	va_start(va, n);
	struct A a;
	while(n--)
	{
		a = va_arg(va, struct A);
		printf("magic: %d, id: %d\n", a.magic, a.id);
        if(IS_CONST(a)) {
            printf("const .\n");
        } else {
            printf("not const.\n");
        }
	}
	va_end(va);
}

int main()
{
	print("rong %s\n", "tao, hello world");
	print("asdfadsf\n");

	const struct A a = {MAGIC, 12};
    struct A b = {MAGIC, 11};
	va_struct(2, a, b);

    if(IS_CONST(a)) {
        printf("a is const .\n");
    } else {
        printf("a is not const.\n");
    }
    if(IS_CONST(1)) {
        printf("b is const .\n");
    } else {
        printf("b is not const.\n");
    }

}

