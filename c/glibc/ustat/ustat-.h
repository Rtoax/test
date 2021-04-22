#include <sys/types.h>
#include <unistd.h>    /* libc[45] */
#include <ustat.h>     /* glibc2 */

//ustat - get file system statistics
int ustat(dev_t dev, struct ustat *ubuf);

