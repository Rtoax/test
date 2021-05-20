
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <time.h>


#define __USE_GNU
#include <sched.h>
#include <ctype.h>
#include <string.h>

//int sched_setaffinity (pid t pid, size t cpusetsize, const cpu set t *cpuset)

int main()
{
    long int ncpu = sysconf (_SC_NPROCESSORS_ONLN);
    int icpu;
    
    cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
    
    CPU_SET(ncpu>1?ncpu-1:1, &cpuset);
    
    int ret = sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    
    printf("ret = %d\n", ret);
    
    int j;
    for(j=0;j<CPU_SETSIZE; j++)
    {
        if(CPU_ISSET(j, &cpuset))
        {
            printf("CPU_SETSIZE = %d, j = %d, cpuset = %d\n", CPU_SETSIZE, j, cpuset);
            CPU_CLR(j, &cpuset);
            printf("CPU_SETSIZE = %d, j = %d, cpuset = %d\n", CPU_SETSIZE, j, cpuset);
        }
    }
    
    icpu = sched_getcpu();
    printf("sched_getcpu ret = %d\n", icpu);
    
    ret = sched_getaffinity(getpid(), sizeof(cpuset), &cpuset);

    for(j=0;j<CPU_SETSIZE; j++)
    {
        if(CPU_ISSET(j, &cpuset))
        {
            printf("CPU_SETSIZE = %d, j = %d, cpuset = %d\n", CPU_SETSIZE, j, cpuset);
        }
    }
    
	return 0;
}
