#include <sys/types.h>
#include <regex.h>

int regexec(const regex_t *preg, const char *string, size_t nmatch, regmatch_t pmatch[], int eflags);

