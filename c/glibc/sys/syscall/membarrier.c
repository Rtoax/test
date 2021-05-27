#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <linux/membarrier.h>




int main()
{
	syscall(SYS_membarrier, MEMBARRIER_CMD_GLOBAL, MEMBARRIER_CMD_FLAG_CPU, 0);


	return 0;
}


