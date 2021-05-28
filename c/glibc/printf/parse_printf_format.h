#include <printf.h>

enum
{				/* C type: */
    PA_INT,			/* int */
    PA_CHAR,			/* int, cast to char */
    PA_WCHAR,			/* wide char */
    PA_STRING,			/* const char *, a '\0'-terminated string */
    PA_WSTRING,			/* const wchar_t *, wide character string */
    PA_POINTER,			/* void * */
    PA_FLOAT,			/* float */
    PA_DOUBLE,			/* double */
    PA_LAST
};


size_t parse_printf_format (const char *__restrict __fmt, size_t __n,
				   int *__restrict __argtypes);

