#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

typedef void (*sighandler_t) (int);

void catchsigint(int signo)
{
    printf("SIGHandler: PID %d\n", getpid());
	exit(1);	
}

int main(void)
{
    sighandler_t handler;
	printf("Main: PID %d\n", getpid());

    handler = signal(SIGINT, catchsigint);
    if (handler == SIG_ERR) 
    {
        perror("signal error");
        exit(1);

    }

    while (1) {
		sleep(1);
	}

    return 0;

}

