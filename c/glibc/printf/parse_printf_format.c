#include <stdio.h>
#include <printf.h>

static void fmt_parse(const char *fmt)
{
    int i;
    int argtypes[1024] = {0};
    
    size_t ret = parse_printf_format(fmt, 1024, argtypes);

    printf("===========================\n");
    printf("%s\n", fmt);
    for(i=0;i<ret;i++) {
        switch(argtypes[i]) {

#define _CASE(i, v, intro) \
        case v:              \
            printf("\t%d is %s\n", i, intro);    \
            break;  

        _CASE(i, PA_INT, "int");
        _CASE(i, PA_INT|PA_FLAG_LONG, "long int");
        _CASE(i, PA_INT|PA_FLAG_SHORT, "short int");
        _CASE(i, PA_INT|PA_FLAG_LONG_LONG, "long long int");

        _CASE(i, PA_CHAR, "char");
        _CASE(i, PA_WCHAR, "wchar");
        _CASE(i, PA_STRING, "string");
        _CASE(i, PA_WSTRING, "wstring");
        _CASE(i, PA_POINTER, "pointer");
        _CASE(i, PA_FLOAT, "float");
        _CASE(i, PA_DOUBLE, "double");
        _CASE(i, PA_DOUBLE|PA_FLAG_LONG_DOUBLE, "long double");
        _CASE(i, PA_LAST, "LAST");


        default:
            printf("\t%d unknown(%d).\n", i, argtypes[i]);
            break;
        }
    }
}


const char *test_fmts[] = {

"1.%d 2.%u 3.%x",
"1.%2d 2.%2u 3.%2x",
"1.%-d 2.%-u 3.%-x",
"1.%-2d 2.%-2u 3.%-2x",

"1.%ld 2.%lu 3.%lx",
"1.%2ld 2.%2lu 3.%2lx",
"1.%-ld 2.%-lu 3.%-lx",
"1.%-2ld 2.%-2lu 3.%-2lx %lld %-lld %2lld %-2lld",

"1.%s 2.%S 3.%c 4.%C",
"1.%-s 2.%-S 3.%-c 4.%-C",
"1.%10s 2.%10S 3.%c 4.%C",
"1.%-10s 2.%-10S 3.%c 4.%C",
"%*.s","%.*s","%*.*s",

"1.%f 2.%lf %llf",
"1.%-f 2.%-lf",
"1.%2f 2.%2lf",
"1.%.2f 2.%.2lf %llf %-llf %2llf %-2llf",

"%10.2f %2.lf %-2.lf %-.10lf",

"%p %x %0x %#02x %#02lx",

"%*.d","%.*d","%*.*d",
"%*.d  %.*d  %*.*d",

NULL,
};

int main()
{
    int i = 0;

    while(test_fmts[i]) {
        fmt_parse(test_fmts[i++]);
    }
}


