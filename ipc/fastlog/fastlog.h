/**
 *  FastLog 低时延/高吞吐 LOG日志系统
 *
 *
 *  Author: Rong Tao
 *  data: 2021年6月2日 - 
 */

#ifndef __fAStLOG_H
#define __fAStLOG_H 1

#include <fastlog_internal.h>


/* 级别 */
enum FASTLOG_LEVEL
{
	FASTLOG_CRIT = 1,
	FASTLOG_ERR,      
	FASTLOG_WARNING, 
	FASTLOG_INFO,      
	FASTLOG_DEBUG,     
	FASTLOGLEVELS_NUM  
};

static const char* FASTLOG_LEVEL_NAME[] = {"CRIT", "ERROR", "WARNING", "NOTICE", "DEBUG"};



//int FAST_LOG(int level, const char *name, const char *format, ...);
#define FAST_LOG(level, name, format, ...) __FAST_LOG(256, level, name, format, ##__VA_ARGS__)
#define FAST_LOG_RAW(size, level, name, format, ...) __FAST_LOG(size, level, name, format, ##__VA_ARGS__)







#endif /*<__fAStLOG_H>*/
