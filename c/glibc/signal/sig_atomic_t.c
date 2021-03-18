#include <signal.h>
#include <stdio.h>

sig_atomic_t flag = 0;

void sigint_handler() {
	flag = 1;
}

int main() {
	signal(SIGINT, sigint_handler);

	while(!flag) {
		printf("Running.\n");
		sleep(1);
	}
	printf("Exit.\n");
}
