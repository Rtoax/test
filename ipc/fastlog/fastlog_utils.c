#include <fastlog.h>

#include <fastlog_utils.h>


typedef enum {
     _ANSI_NONE,
     _ANSI_GRAY,
     _ANSI_CYAN,
     _ANSI_PURPLE,
     _ANSI_BLUE,
     _ANSI_YELLOW,
     _ANSI_GREEN,
     _ANSI_RED,
     _ANSI_BOLD ,
     _ANSI_SHINE,
     _ANSI_BLINK,
     _ANSI_RESET,
} color_type;
     
#define _ANSI_NONE_STR       ""
#define _ANSI_GRAY_STR       "\x1b[1;37;40m"
#define _ANSI_CYAN_STR       "\x1b[1;36m"
#define _ANSI_PURPLE_STR     "\x1b[1;35m"
#define _ANSI_BLUE_STR       "\x1b[1;34m"
#define _ANSI_YELLOW_STR     "\x1b[1;33m"
#define _ANSI_GREEN_STR      "\x1b[1;32m"
#define _ANSI_RED_STR        "\x1b[1;31m"
#define _ANSI_BOLD_STR       "\x1b[1m"
#define _ANSI_SHINE_STR      "\x1b[1;35m" /*"\x1b[1;5;35m"*/
#define _ANSI_BLINK_STR      "\x1b[1;4;5m"
     
#define _ANSI_RESET_STR      "\x1b[0m"

#define _ITEM(e) [e] = e##_STR
const static char *COLORS[] = {
    _ITEM(_ANSI_NONE),
    _ITEM(_ANSI_GRAY),
    _ITEM(_ANSI_CYAN),
    _ITEM(_ANSI_PURPLE),
    _ITEM(_ANSI_BLUE),
    _ITEM(_ANSI_YELLOW),
    _ITEM(_ANSI_GREEN),
    _ITEM(_ANSI_RED),
    _ITEM(_ANSI_BOLD),
    _ITEM(_ANSI_SHINE),
    _ITEM(_ANSI_BLINK),
    _ITEM(_ANSI_RESET),
};
#undef _ITEM


static inline int _color_vprint(FILE *fp, color_type color, const char *fmt, va_list arg);

int fp_gray(FILE *fp, const char *fmt, ...)
{
    va_list arg1;
    va_start(arg1, fmt);
    int n = _color_vprint(fp, _ANSI_GRAY, fmt, arg1);
    va_end(arg1);
    return n;
}

//TODO


static inline int _color_vprint(FILE *fp, color_type color, const char *fmt, va_list arg)
{
    int n = 0;        

    if(color != _ANSI_NONE)
        fprintf(fp, "%s", COLORS[color]);

    n += vfprintf(fp, fmt, arg);

    if(color != _ANSI_NONE)
        fprintf(fp, "%s", _ANSI_RESET);


    return n;
}

