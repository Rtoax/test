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

/**
 *  FastLog API
 *
 *  level   日志级别(enum FASTLOG_LEVEL)
 *  name    用户字符串(模块名，prefix等)
 *  format  printf(fmt, ...)'s fmt
 *  ...     printf(fmt, ...)'s ...
 */
void FAST_LOG(int level, char *name, char *format, ...);


const char *strlevel(enum FASTLOG_LEVEL level);
const char *strlevel_color(enum FASTLOG_LEVEL level);




/**
 *  FastLog 初始化
 *
 *  nr_logfile      最大日志文件数
 *  log_file_size   单个日志文件大小(bytes)
 *
 */
void fastlog_init(size_t nr_logfile, size_t log_file_size);
void fastlog_exit();


#define FAST_LOG(level, name, format, ...) __FAST_LOG(level, name, format, ##__VA_ARGS__)


#endif /*<__fAStLOG_H>*/
