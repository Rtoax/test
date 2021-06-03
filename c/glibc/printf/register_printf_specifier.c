#include <stdio.h>
#include <printf.h>

//int iinfo = 0;
//struct printf_info infos[10];

int my_printf_function(FILE *__stream, const struct printf_info *__info, const void *const *__args)
{
    printf("call %s\n", __func__);
    printf_size(stderr, __info, __args);
//    memcpy(&infos[iinfo++], __info, sizeof(struct printf_info));
}

int my_printf_arginfo_size_function(const struct printf_info *__info,
					  size_t __n, int *__argtypes,
					  int *__size)
{
    printf("call %s\n", __func__);
    printf_size_info(__info, __n, __argtypes);
//    memcpy(&infos[iinfo++], __info, sizeof(struct printf_info));
    
}


int main()
{
    register_printf_specifier('d', my_printf_function, my_printf_arginfo_size_function);

    printf("%d %ld\n", 1, 2);
    
    register_printf_specifier('d', NULL, NULL);
    printf("%d %ld\n", 1, 2);
}               
