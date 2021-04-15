#include <stdio.h>
#include <glob.h>

int main() {

	//下面的代码等同于 这个 ls 指令
	// ls -l *.c ../*.c
	glob_t globbuf;

    globbuf.gl_offs = 2;
    glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
    glob("../*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
    globbuf.gl_pathv[0] = "ls";
    globbuf.gl_pathv[1] = "-l";
    execvp("ls", &globbuf.gl_pathv[0]);

}
