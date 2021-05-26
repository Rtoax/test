#include "printf.h"

static void fmt_parse(const char *fmt)
{
    int i;
    int argtypes[1024] = {0};
    
    size_t ret = parse_printf_format(fmt, 1024, argtypes);

    printf("===========================\n");
    printf("%s\n", fmt);
    for(i=0;i<ret;i++) {
        switch(argtypes[i]) {
        case PA_INT:			/* int */
            printf("\t%d is %s\n", i, "PA_INT");
            break;
        case PA_INT|PA_FLAG_LONG:			/* long int */
            printf("\t%d is %s\n", i, "PA_INT long");
            break;
        case PA_CHAR:			/* int, cast to char */
            printf("\t%d is %s\n", i, "PA_CHAR");
            break;
        case PA_WCHAR:			/* wide char */
            printf("\t%d is %s\n", i, "PA_WCHAR");
            break;
        case PA_STRING:			/* const char *, a '\0'-terminated string */
            printf("\t%d is %s\n", i, "PA_STRING");
            break;
        case PA_WSTRING:			/* const wchar_t *, wide character string */
            printf("\t%d is %s\n", i, "PA_WSTRING");
            break;
        case PA_POINTER:			/* void * */
            printf("\t%d is %s\n", i, "PA_POINTER");
            break;
        case PA_FLOAT:			/* float */
            printf("\t%d is %s\n", i, "PA_FLOAT");
            break;
        case PA_DOUBLE:			/* double */
            printf("\t%d is %s\n", i, "PA_DOUBLE");
            break;
        case PA_LAST:
            printf("\t%d is %s\n", i, "PA_LAST");
            break;
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
"1.%-2ld 2.%-2lu 3.%-2lx",

"1.%s 2.%S 3.%c 4.%C",
"1.%-s 2.%-S 3.%-c 4.%-C",
"1.%10s 2.%10S 3.%c 4.%C",
"1.%-10s 2.%-10S 3.%c 4.%C",

"1.%f 2.%lf",
"1.%-f 2.%-lf",
"1.%2f 2.%2lf",
"1.%.2f 2.%.2lf",
NULL,
};

int main()
{
    int i = 0;

    while(test_fmts[i]) {
        fmt_parse(test_fmts[i++]);
    }
}

