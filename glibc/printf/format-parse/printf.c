#include <limits.h>
#include <string.h>
#include <assert.h>
#include <readline/chardefs.h>  //ISDIGIT()

#include <printf.h>
#include <printf-parse.h>

//#define COMPILE_WPRINTF

#define MAX(a, b) (((a) > (b))?(a) : (b))
#define MIN(a, b) (((a) > (b))?(b) : (a))

//#define WIDE

#ifndef WIDE
# define STR_LEN strlen
# define STR_CMP strcmp
# define SPRINT snprintf
# define CHAR_T char
# define PRINT printf
# define L_(Str) Str
# define S "%s"
#else
# define STR_LEN wcslen
# define SPRINT swprintf
# define STR_CMP wcscmp
# define CHAR_T wchar_t
# define PRINT wprintf
# define L_(Str) L##Str
# define S "%ls"
#endif


static char *__strchrnul(const char *s, int c)
{
  char *result = strchr (s, c);
  if (result == NULL)
    result = strchr (s, '\0');
  return result;
}
/* Find the first occurrence of WC in WCS.  */
wchar_t *__wcschrnul(const wchar_t *wcs, const wchar_t wc)
{
  while (*wcs != L'\0')
    if (*wcs == wc)
      break;
    else
      ++wcs;

  return (wchar_t *) wcs;
}


const unsigned char *__find_specmb(const unsigned char *format)
{
    return (const unsigned char *) __strchrnul ((const char *) format, '%');
}

const unsigned int *__find_specwc(const unsigned int *format)
{
  return (const unsigned int *) __wcschrnul ((const wchar_t *) format, L'%');
}


static int read_int(const UCHAR_T * *pstr)
{
  int retval = **pstr - L_('0');

  while (ISDIGIT (*++(*pstr)))
    if (retval >= 0)
      {
	if (INT_MAX / 10 < retval)
	  retval = -1;
	else
	  {
	    int digit = **pstr - L_('0');

	    retval *= 10;
	    if (INT_MAX - digit < retval)
	      retval = -1;
	    else
	      retval += digit;
	  }
      }

  return retval;
}



size_t __parse_one_specmb (const UCHAR_T *format, size_t posn,
		    struct printf_spec *spec, size_t *max_ref_arg)
{
    unsigned int n;
    size_t nargs = 0;

    /* Skip the '%'.  */
    ++format;

    /* Clear information structure.  */
    spec->data_arg = -1;
    spec->info.alt = 0;
    spec->info.space = 0;
    spec->info.left = 0;
    spec->info.showsign = 0;
    spec->info.group = 0;
    spec->info.i18n = 0;
    spec->info.extra = 0;
    spec->info.pad = ' ';
    spec->info.wide = sizeof (UCHAR_T) > 1;
    spec->info.is_binary128 = 0;

    /* Test for positional argument.  */
    if (ISDIGIT (*format))
    {
        const UCHAR_T *begin = format;

        n = read_int (&format);

        if (n != 0 && *format == L_('$'))
        /* Is positional parameter.  */
        {
            ++format;		/* Skip the '$'.  */
            if (n != -1)
            {
                spec->data_arg = n - 1;
                *max_ref_arg = MAX (*max_ref_arg, n);
            }
        }
        else
            /* Oops; that was actually the width and/or 0 padding flag.
            Step back and read it again.  */
            format = begin;
    }

  /* Check for spec modifiers.  */
    do
    {
        switch (*format)
        {
        case L_(' '):
            /* Output a space in place of a sign, when there is no sign.  */
            spec->info.space = 1;
            continue;
        case L_('+'):
            /* Always output + or - for numbers.  */
            spec->info.showsign = 1;
            continue;
        case L_('-'):
            /* Left-justify things.  */
            spec->info.left = 1;
            continue;
        case L_('#'):
            /* Use the "alternate form":
            Hex has 0x or 0X, FP always has a decimal point.  */
            spec->info.alt = 1;
            continue;
        case L_('0'):
            /* Pad with 0s.  */
            spec->info.pad = '0';
            continue;
        case L_('\''):
            /* Show grouping in numbers if the locale information
            indicates any.  */
            spec->info.group = 1;
            continue;
        case L_('I'):
            /* Use the internationalized form of the output.  Currently
            means to use the `outdigits' of the current locale.  */
            spec->info.i18n = 1;
            continue;
        default:
            break;
        }
        break;
    }while (*++format);

    if (spec->info.left)
        spec->info.pad = ' ';

    /* Get the field width.  */
    spec->width_arg = -1;
    spec->info.width = 0;
    
    if (*format == L_('*'))
    {
        /* The field width is given in an argument.
        A negative field width indicates left justification.  */
        const UCHAR_T *begin = ++format;

        if (ISDIGIT (*format))
        {
            /* The width argument might be found in a positional parameter.  */
            n = read_int (&format);

            if (n != 0 && *format == L_('$'))
            {
                if (n != -1)
                {
                    spec->width_arg = n - 1;
                    *max_ref_arg = MAX (*max_ref_arg, n);
                }
                ++format;		/* Skip '$'.  */
            }
        }

        if (spec->width_arg < 0)
        {
            /* Not in a positional parameter.  Consume one argument.  */
            spec->width_arg = posn++;
            ++nargs;
            format = begin;	/* Step back and reread.  */
        }
    }
    else if (ISDIGIT (*format))
    {
        int n = read_int (&format);

        /* Constant width specification.  */
        if (n != -1)
            spec->info.width = n;
    }
    
    /* Get the precision.  */
    spec->prec_arg = -1;
    /* -1 means none given; 0 means explicit 0.  */
    spec->info.prec = -1;
    
    if (*format == L_('.'))
    {
        ++format;
        if (*format == L_('*'))
        {
            /* The precision is given in an argument.  */
            const UCHAR_T *begin = ++format;

            if (ISDIGIT (*format))
            {
                n = read_int (&format);

                if (n != 0 && *format == L_('$'))
                {
                    if (n != -1)
                    {
                        spec->prec_arg = n - 1;
                        *max_ref_arg = MAX (*max_ref_arg, n);
                    }
                    ++format;
                }
            }

            if (spec->prec_arg < 0)
            {
                /* Not in a positional parameter.  */
                spec->prec_arg = posn++;
                ++nargs;
                format = begin;
            }
        }
        else if (ISDIGIT (*format))
        {
            int n = read_int (&format);

            if (n != -1)
            spec->info.prec = n;
        }
        else
            /* "%.?" is treated like "%.0?".  */
            spec->info.prec = 0;
    }

    /* Check for type modifiers.  */
    spec->info.is_long_double = 0;
    spec->info.is_short = 0;
    spec->info.is_long = 0;
    spec->info.is_char = 0;
    spec->info.user = 0;

    switch (*format++)
    {
    case L_('h'):
        /* ints are short ints or chars.  */
        if (*format != L_('h'))
            spec->info.is_short = 1;
        else
        {
            ++format;
            spec->info.is_char = 1;
        }
        break;
    case L_('l'):
        /* ints are long ints.  */
        spec->info.is_long = 1;
        if (*format != L_('l'))
            break;
        ++format;
    /* FALLTHROUGH */
    case L_('L'):
    /* doubles are long doubles, and ints are long long ints.  */
    case L_('q'):
        /* 4.4 uses this for long long.  */
        spec->info.is_long_double = 1;
        break;
    case L_('z'):
    case L_('Z'):
        /* ints are size_ts.  */
        assert (sizeof (size_t) <= sizeof (unsigned long long int));
#if LONG_MAX != LONG_LONG_MAX
        spec->info.is_long_double = (sizeof (size_t) > sizeof (unsigned long int));
#endif
        spec->info.is_long = sizeof (size_t) > sizeof (unsigned int);
        break;
    case L_('t'):
        assert (sizeof (ptrdiff_t) <= sizeof (long long int));
#if LONG_MAX != LONG_LONG_MAX
        spec->info.is_long_double = (sizeof (ptrdiff_t) > sizeof (long int));
#endif
        spec->info.is_long = sizeof (ptrdiff_t) > sizeof (int);
        break;
    case L_('j'):
        assert (sizeof (uintmax_t) <= sizeof (unsigned long long int));
#if LONG_MAX != LONG_LONG_MAX
        spec->info.is_long_double = (sizeof (uintmax_t) > sizeof (unsigned long int));
#endif
        spec->info.is_long = sizeof (uintmax_t) > sizeof (unsigned int);
        break;
    default:
        /* Not a recognized modifier.  Backup.  */
        --format;
        break;
    }

    /* Get the format specification.  */
    spec->info.spec = (wchar_t) *format++;
    spec->size = -1;
  
    if(1)
    {
        /* Find the data argument types of a built-in spec.  */
        spec->ndata_args = 1;

        switch (spec->info.spec)
        {
        case L'i':
        case L'd':
        case L'u':
        case L'o':
        case L'X':
        case L'x':
#if LONG_MAX != LONG_LONG_MAX
            if (spec->info.is_long_double)
                spec->data_arg_type = PA_INT|PA_FLAG_LONG_LONG;
            else
#endif
            if (spec->info.is_long)
                spec->data_arg_type = PA_INT|PA_FLAG_LONG;
            else if (spec->info.is_short)
                spec->data_arg_type = PA_INT|PA_FLAG_SHORT;
            else if (spec->info.is_char)
                spec->data_arg_type = PA_CHAR;
            else
                spec->data_arg_type = PA_INT;
            break;
        case L'e':
        case L'E':
        case L'f':
        case L'F':
        case L'g':
        case L'G':
        case L'a':
        case L'A':
            if (spec->info.is_long_double)
                spec->data_arg_type = PA_DOUBLE|PA_FLAG_LONG_DOUBLE;
            else
                spec->data_arg_type = PA_DOUBLE;
            break;
        case L'c':
            spec->data_arg_type = PA_CHAR;
            break;
        case L'C':
            spec->data_arg_type = PA_WCHAR;
            break;
        case L's':
            spec->data_arg_type = PA_STRING;
            break;
        case L'S':
            spec->data_arg_type = PA_WSTRING;
            break;
        case L'p':
            spec->data_arg_type = PA_POINTER;
            break;
        case L'n':
            spec->data_arg_type = PA_INT|PA_FLAG_PTR;
            break;

        case L'm':
        default:
            /* An unknown spec will consume no args.  */
            spec->ndata_args = 0;
            break;
        }
    }

    if (spec->data_arg == -1 && spec->ndata_args > 0)
    {
        /* There are args consumed, but no positional spec.  Use the
        next sequential arg position.  */
        spec->data_arg = posn;
        nargs += spec->ndata_args;
    }

    if (spec->info.spec == L'\0')
        /* Format ended before this spec was complete.  */
        spec->end_of_fmt = spec->next_fmt = format - 1;
    else
    {
        /* Find the next format spec.  */
        spec->end_of_fmt = format;
#ifdef COMPILE_WPRINTF
        spec->next_fmt = __find_specwc (format);
#else
        spec->next_fmt = __find_specmb (format);
#endif
    }

  return nargs;
}

size_t parse_printf_format (const char *fmt, size_t n, int *argtypes)
{
    size_t nargs;			/* Number of arguments.  */
    size_t max_ref_arg;		/* Highest index used in a positional arg.  */
    struct printf_spec spec;
    const unsigned char *f = (const unsigned char *) fmt;

    nargs = 0;
    max_ref_arg = 0;

    /* Search for format specifications.  */
    for (f = __find_specmb (f); *f != '\0'; f = spec.next_fmt)
    {
        /* Parse this spec.  */
        nargs += __parse_one_specmb (f, nargs, &spec, &max_ref_arg);

        /* If the width is determined by an argument this is an int.  */
        if (spec.width_arg != -1 && (size_t) spec.width_arg < n)
            argtypes[spec.width_arg] = PA_INT;

        /* If the precision is determined by an argument this is an int.  */
        if (spec.prec_arg != -1 && (size_t) spec.prec_arg < n)
            argtypes[spec.prec_arg] = PA_INT;

        if ((size_t) spec.data_arg < n)
        switch (spec.ndata_args)
        {
            case 0:		/* No arguments.  */
                break;
            case 1:		/* One argument; we already have the type.  */
                argtypes[spec.data_arg] = spec.data_arg_type;
                break;
            default:
                printf("No case %d.\n", spec.ndata_args);
                /* We have more than one argument for this format spec.  We must
                call the arginfo function again to determine all the types.  */
//                (void) (*__printf_arginfo_table[spec.info.spec])
//                            (&spec.info, n - spec.data_arg, &argtypes[spec.data_arg], &spec.size);
                break;
        }
    }

    return MAX (nargs, max_ref_arg);
}

