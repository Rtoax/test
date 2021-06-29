#ifndef __MODULE3_H
#define __MODULE3_H

#define M3_CRIT(fmt...)     FAST_LOG(FASTLOG_CRIT, MODULE(MODULE_3), fmt)
#define M3_ERR(fmt...)      FAST_LOG(FASTLOG_ERR, MODULE(MODULE_3), fmt)
#define M3_WARN(fmt...)     FAST_LOG(FASTLOG_WARNING, MODULE(MODULE_3), fmt)
#define M3_INFO(fmt...)     FAST_LOG(FASTLOG_INFO, MODULE(MODULE_3), fmt)
#define M3_DEBUG(fmt...)    FAST_LOG(FASTLOG_DEBUG, MODULE(MODULE_3), fmt)


void module3_init();
void *task_module3(void*param);

#endif



