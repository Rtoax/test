#include <stdio.h>
#include <printf.h>

int my_printf_function(FILE *__stream, const struct printf_info *__info, const void *const *__args)
{
    printf("call %s\n", __func__);
    printf_size(stderr, __info, __args);
}

int my_printf_arginfo_function(const struct printf_info *__info,
				     size_t __n, int *__argtypes)
{
    printf("call %s\n", __func__);
    printf_size_info(__info, __n, __argtypes);
}


int main()
{
    register_printf_function('d', my_printf_function, my_printf_arginfo_function);

    printf("%d %ld\n", 1 ,2 );
}               

