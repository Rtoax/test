#ifndef __PRINTF_PARSE_H
#define __PRINTF_PARSE_H 1

#include <printf.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#define UCHAR_T unsigned char

struct printf_spec
{
    /* Information parsed from the format spec.  */
    struct printf_info info;

    /* Pointers into the format string for the end of this format
    spec and the next (or to the end of the string if no more).  */
    const UCHAR_T *end_of_fmt, *next_fmt;

    /* Position of arguments for precision and width, or -1 if `info' has
    the constant value.  */
    int prec_arg, width_arg;

    int data_arg;		/* Position of data argument.  */
    int data_arg_type;		/* Type of first argument.  */
    /* Number of arguments consumed by this format specifier.  */
    size_t ndata_args;
    /* Size of the parameter for PA_USER type.  */
    int size;
};

/* The various kinds off arguments that can be passed to printf.  */
union printf_arg
{
    wchar_t pa_wchar;
    int pa_int;
    long int pa_long_int;
    long long int pa_long_long_int;
    unsigned int pa_u_int;
    unsigned long int pa_u_long_int;
    unsigned long long int pa_u_long_long_int;
    double pa_double;
    long double pa_long_double;
#if __HAVE_FLOAT128_UNLIKE_LDBL
    _Float128 pa_float128;
#endif
    const char *pa_string;
    const wchar_t *pa_wstring;
    void *pa_pointer;
    void *pa_user;
};


#endif /*<__PRINTF_PARSE_H>*/
