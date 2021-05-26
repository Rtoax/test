#ifndef __PRINTF_H
#define __PRINTF_H 1


#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>


struct printf_info
{
    int prec;			/* Precision.  */
    int width;			/* Width.  */
    wchar_t spec;			/* Format letter.  */
    unsigned int is_long_double:1;/* L flag.  */
    unsigned int is_short:1;	/* h flag.  */
    unsigned int is_long:1;	/* l flag.  */
    unsigned int alt:1;		/* # flag.  */
    unsigned int space:1;		/* Space flag.  */
    unsigned int left:1;		/* - flag.  */
    unsigned int showsign:1;	/* + flag.  */
    unsigned int group:1;		/* ' flag.  */
    unsigned int extra:1;		/* For special use.  */
    unsigned int is_char:1;	/* hh flag.  */
    unsigned int wide:1;		/* Nonzero for wide character streams.  */
    unsigned int i18n:1;		/* I flag.  */
    unsigned int is_binary128:1;	/* Floating-point argument is ABI-compatible
                                    with IEC 60559 binary128.  */
    unsigned int __pad:3;		/* Unused so far.  */
    unsigned short int user;	/* Bits for user-installed modifiers.  */
    wchar_t pad;			/* Padding character.  */
};


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

/* Flag bits that can be set in a type returned by `parse_printf_format'.  */
#define	PA_FLAG_MASK		0xff00
#define	PA_FLAG_LONG_LONG	(1 << 8)
#define	PA_FLAG_LONG_DOUBLE	PA_FLAG_LONG_LONG
#define	PA_FLAG_LONG		(1 << 9)
#define	PA_FLAG_SHORT		(1 << 10)
#define	PA_FLAG_PTR		(1 << 11)


extern size_t parse_printf_format(const char * __fmt, size_t __n, int * __argtypes);


#endif /*<__PRINTF_H>*/
