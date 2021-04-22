
#include <sys/types.h>
#include <attr/xattr.h>

//flags
#define XATTR_CREATE  0x1       /* set value, fail if attr already exists */
#define XATTR_REPLACE 0x2       /* set value, fail if attr does not exist */


//setxattr, lsetxattr, fsetxattr - set an extended attribute value
int setxattr (const char *path, const char *name,
               const void *value, size_t size, int flags);
int lsetxattr (const char *path, const char *name,
               const void *value, size_t size, int flags);
int fsetxattr (int filedes, const char *name,
               const void *value, size_t size, int flags);

