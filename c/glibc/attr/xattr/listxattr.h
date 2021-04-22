#include <sys/types.h>
#include <attr/xattr.h>

//listxattr, llistxattr, flistxattr - list extended attribute names
ssize_t listxattr (const char *path,
                    char *list, size_t size);
ssize_t llistxattr (const char *path,
                    char *list, size_t size);
ssize_t flistxattr (int filedes,
                    char *list, size_t size);

