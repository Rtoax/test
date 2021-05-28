#ifndef FASTLOG_H
#define FASTLOG_H


#include <stdio.h>
#include <stdarg.h>


enum LogLevel {
    SILENT_LOG_LEVEL = 0,
    ERROR,
    WARNING,
    NOTICE,
    INFO,
    DEBUG,
    NUM_LOG_LEVELS
};


static const char * const _level[] = {
	"SILENT_LOG_LEVEL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
	NULL,
};

#define FASTLOG(level, fmt...) do { \
	fastLog("FASTLOG", level, __FILE__, __LINE__, fmt); \
}while(0)

static int fastLog(const char *prefix, int level, const char *file, int line, const char *fmt, ...)
{
	int ret = 0;
	printf("[%s][%s][%s:%d] ", prefix, _level[level], file, line);
	va_list va;
	va_start(va, fmt);
	ret = vprintf(fmt, va);
	va_end(va);
	return ret;
}


#endif /* FASTLOG_H */
