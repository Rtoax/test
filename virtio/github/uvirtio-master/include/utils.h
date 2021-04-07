#ifndef __log_h
#define __log_h 1

#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdarg.h>


enum {
	L_CRIT,
	L_WARNING,
	L_NOTICE,
	L_INFO,
	L_DEBUG
};


#ifdef NODEBUG
#define lerror(fmt...)
#define linfo(fmt...) 
#define ldebug(fmt...)
#define lwarning(fmt...) 

#else
#define lerror(fmt...)  ___log_message(__FILE__, __func__, __LINE__,"virtio", L_CRIT, fmt)
#define linfo(fmt...)  ___log_message(__FILE__, __func__, __LINE__,"virtio",   L_INFO, fmt)
#define ldebug(fmt...)  ___log_message(__FILE__, __func__, __LINE__,"virtio",   L_DEBUG, fmt)
#define lwarning(fmt...)  ___log_message(__FILE__, __func__, __LINE__,"virtio",   L_WARNING, fmt)
#endif

#ifndef _unused
#define _unused             __attribute__((unused))
#endif



static const char*_unused ___log_str[] = {
	[L_CRIT] = "CRITICAL",
	[L_WARNING] = "WARNING",
	[L_NOTICE] = "NOTICE",
	[L_INFO] = "INFO",
	[L_DEBUG] = "DEBUG"
};



static void _unused ___log_message(char *file, const char *func, int line, char *name, int priority, const char *format, ...)
{
	va_list _unused vl;

	va_start(vl, format);
    #if 0
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("%d-%02d-%02d %02d:%02d:%02d [easycwmp] %s - ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, log_str[priority]);
    #else
    printf("[%s][%s %d] ", name, func, line);
    #endif
    vprintf(format, vl);
	va_end(vl);
}

#endif
