#include <sys/types.h>
#include <linux/userfaultfd.h>

//userfaultfd机制可以让用户来处理缺页，可以在用户空间定义自己的page fau handler
int userfaultfd(int flags);

