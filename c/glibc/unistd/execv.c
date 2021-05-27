#include <unistd.h>
#include <stdio.h>

int main()
{
    char *const argv [] = {
        "-a",
        };

	execv("/bin/ls", argv);

    printf("\n\nexecve done.\n\n");
}

