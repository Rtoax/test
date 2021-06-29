#ifndef __MODULE1_H
#define __MODULE1_H

#include <fastlog.h>


#define M1_CRIT(fmt...)     FAST_LOG(FASTLOG_CRIT, MODULE(MODULE_1), fmt)
#define M1_ERR(fmt...)      FAST_LOG(FASTLOG_ERR, MODULE(MODULE_1), fmt)
#define M1_WARN(fmt...)     FAST_LOG(FASTLOG_WARNING, MODULE(MODULE_1), fmt)
#define M1_INFO(fmt...)     FAST_LOG(FASTLOG_INFO, MODULE(MODULE_1), fmt)
#define M1_DEBUG(fmt...)    FAST_LOG(FASTLOG_DEBUG, MODULE(MODULE_1), fmt)



void module1_init();
void *task_module1(void*param);

#endif

