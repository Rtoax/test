#include <printf.h>

//使用用于数字的数量级的缩写（'k' 表示千，'m' 表示兆等）打印浮点值。
//如果格式说明符是大写字符，则使用 1000 的幂。 否则为 1024 的幂。
int printf_size (FILE *__restrict __fp,
        			const struct printf_info *__info,
        			const void *const *__restrict __args);

