#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <fnmatch.h>

#include <rte_log.h>
#include <rte_per_lcore.h>

/*
 * Generates a log message The message will be sent in the stream
 * defined by the previous call to rte_openlog_stream().
 */
int
rte_vlog(uint32_t level, uint32_t logtype, const char *format, va_list ap)
{
	FILE *f = stderr;
	int ret;

	ret = vfprintf(f, format, ap);
	fflush(f);
	return ret;
}


/*
 * Generates a log message The message will be sent in the stream
 * defined by the previous call to rte_openlog_stream().
 * No need to check level here, done by rte_vlog().
 */
int
rte_log(uint32_t level, uint32_t logtype, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = rte_vlog(level, logtype, format, ap);
	va_end(ap);
	return ret;
}


int rte_debug(const char *func, const int line, const char *format, ...)
{
	va_list ap;
	int ret = 0;

	va_start(ap, format);
    
	ret += fprintf(stderr, "[%s %d] ", func, line);
	ret += vfprintf(stderr, format, ap);
    
	fflush(stderr);
	va_end(ap);
	return ret;
}

