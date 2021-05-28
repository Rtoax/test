#include "header.h"

int i = 0;

int main(int argc, char *argv[], char *env[])
{
	MY_LOG(INFO, "Hello World. %d\n", i);
	return 0;
}
