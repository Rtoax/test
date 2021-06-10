#include <time.h>
#include <sys/time.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    char buffer[40];
    struct timeval tv;
        
    gettimeofday(&tv, NULL);

    time_t t = time(NULL);

    strftime(buffer, 40, "Current date/time: %m-%d-%Y/%T", localtime(&tv.tv_sec));
    printf("%s\n",buffer);
    strftime(buffer, 40, "Current date/time: %m-%d-%Y/%T", localtime(&t));
    printf("%s\n",buffer);

    return 0;
}
