#include <printf.h>


//过时的接口类似于 register_printf_specifier。 它只能处理基本数据类型，因为 ARGINFO 回调不返回有关用户定义类型大小的信息。
int register_printf_function(int __spec, printf_function __func,
				     printf_arginfo_function __arginfo);

