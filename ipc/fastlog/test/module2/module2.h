#ifndef __MODULE2_H
#define __MODULE2_H



#define M2_CRIT(fmt...)     FAST_LOG(FASTLOG_CRIT, MODULE(MODULE_2), fmt)
#define M2_ERR(fmt...)      FAST_LOG(FASTLOG_ERR, MODULE(MODULE_2), fmt)
#define M2_WARN(fmt...)     FAST_LOG(FASTLOG_WARNING, MODULE(MODULE_2), fmt)
#define M2_INFO(fmt...)     FAST_LOG(FASTLOG_INFO, MODULE(MODULE_2), fmt)
#define M2_DEBUG(fmt...)    FAST_LOG(FASTLOG_DEBUG, MODULE(MODULE_2), fmt)


void module2_init();
void *task_module2(void*param);


#endif


