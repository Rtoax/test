#include <printf.h>


//注册要调用的 FUNC 以格式化 SPEC 说明符； 必须指定 ARGINFO 以确定 SPEC 转换需要多少个参数以及它们的类型。
int register_printf_specifier(int __spec, printf_function __func,
				      printf_arginfo_size_function __arginfo);

