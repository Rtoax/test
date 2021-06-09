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
#include <fastlog_staging_buffer.h>


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
#define FAST_LOG(level, name, format, ...) __FAST_LOG(level, name, format, ##__VA_ARGS__)


void fastlog_init();
void fastlog_exit();


#endif /*<__fAStLOG_H>*/
