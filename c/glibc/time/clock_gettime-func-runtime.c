#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


static int timespec_check(struct timespec *t)
{
    if((t->tv_nsec <0 ) || (t->tv_nsec >= 1000000000))
        return -1;

    return 0;
}

static void timespec_sub(struct timespec *t1,  struct timespec *t2)
{
    if (timespec_check(t1) < 0) {
        fprintf(stderr, "invalid time #1: %lld.%.9ld.\n",
            (long long) t1->tv_sec,t1->tv_nsec);
        return;
    }
    if (timespec_check(t2) < 0) {
        fprintf(stderr, "invalid time #2: %lld.%.9ld.\n",
            (long long) t2->tv_sec,t2->tv_nsec);
        return;
    }

    t1->tv_sec -= t2->tv_sec;
    t1->tv_nsec -= t2->tv_nsec;
    if (t1->tv_nsec >= 1000000000)
    {
        t1->tv_sec++;
        t1->tv_nsec -= 1000000000;
    }
    else if (t1->tv_nsec < 0)
    {
        t1->tv_sec--;
        t1->tv_nsec += 1000000000;
    }
}

int main()
{
    int rc;
    int count = 100;
    long t_time_n = 0;  //nano secend
    long t_time_s = 0;  //secnd
    struct timespec ts_start, ts_end;


    while (count--) {

        rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
        usleep(200);

        rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);                                                         

        timespec_sub(&ts_end, &ts_start);
        t_time_n += ts_end.tv_nsec;
        t_time_s += ts_end.tv_sec;

        #if 0
        printf("CLOCK_MONOTONIC reports %ld.%09ld seconds\n", 
                ts_end.tv_sec, ts_end.tv_nsec);     
        #endif
    }
    printf("** Total time %lds + %ld nsec\n",t_time_s,t_time_n);
}

