#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

void sig_handler(int signum) {
	switch(signum) {
		case SIGPIPE:
			printf("Broken Pipe.\n");
		break;
		default:
		break;
	}
}

int main() {
	int fds[2];

	signal(SIGPIPE, sig_handler);
	pipe(fds);

	close(fds[0]);
	write(fds[1], "hello", 5);
}
