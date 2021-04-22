#include <sys/types.h>
#include <attr/xattr.h>

//getxattr, lgetxattr, fgetxattr - retrieve an extended attribute value
ssize_t getxattr (const char *path, const char *name,
                    void *value, size_t size);
ssize_t lgetxattr (const char *path, const char *name,
                    void *value, size_t size);
ssize_t fgetxattr (int filedes, const char *name,
                    void *value, size_t size);

