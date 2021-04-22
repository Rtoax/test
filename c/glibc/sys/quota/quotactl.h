//quotactl - manipulate disk quotas(定额; 限额; 配额; 定量; 指标;)

#include <sys/quota.h>
#include <xfs/xqm.h>

//磁盘的使用和限额
int quotactl(int cmd, const char *special, int id, caddr_t addr);

