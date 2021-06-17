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
    FASTLOGLEVEL_ALL = 0,
	FASTLOG_CRIT = 1,
	FASTLOG_ERR,      
	FASTLOG_WARNING, 
	FASTLOG_INFO,      
	FASTLOG_DEBUG,     
	FASTLOGLEVELS_NUM  
};



const char *strlevel(enum FASTLOG_LEVEL level);
const char *strlevel_color(enum FASTLOG_LEVEL level);


//int FAST_LOG(int level, const char *name, const char *format, ...);
#define FAST_LOG(level, name, format, ...) __FAST_LOG(level, name, format, ##__VA_ARGS__)


void fastlog_init();
void fastlog_exit();


#endif /*<__fAStLOG_H>*/
