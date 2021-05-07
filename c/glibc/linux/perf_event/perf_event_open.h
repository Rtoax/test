#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

//set up performance monitoring
int perf_event_open(struct perf_event_attr *attr,
                   pid_t pid, int cpu, int group_fd,
                   unsigned long flags);
