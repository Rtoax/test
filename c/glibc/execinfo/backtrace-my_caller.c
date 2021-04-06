#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BACK_TRACE_BUFF_SIZE 1024

long my_caller(char *name,int size,unsigned int depth) 
{
    int j=0, nptrs=0;
    void *buffer[BACK_TRACE_BUFF_SIZE];
    char **strings;
    char *start = NULL,*end= NULL,n = 0;

    nptrs = backtrace(buffer, BACK_TRACE_BUFF_SIZE);
    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
       would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        return -1;
    }

//    for(j=0;j<nptrs;j++) {
//        printf("%s\n", strings[j]);
//    }

    j = (nptrs > 2 ? 2 : 0);
    j+=depth;

    if(j >= nptrs)
    {
        free(strings);
//        ASSERT(0);
        return -1;
    }

    start = strstr(strings[j],"(")+1;
    end = strstr(strings[j],"+");

    n = end - start + 1;
    n = n > size ? size : n;
    snprintf(name,n,"%s",start);

    free(strings);
    return 0;
}

int demo1()
{
    char caller[1024] = {0};

    my_caller(caller, sizeof(caller)-1, 0);
    printf("%s caller is %s\n", __func__, caller);
}


int demo2()
{
    char caller[1024] = {0};

    my_caller(caller, sizeof(caller)-1, 0);
    printf("%s caller is %s\n", __func__, caller);
    demo1();
}

int main()
{
    demo1();
    demo2();

}

