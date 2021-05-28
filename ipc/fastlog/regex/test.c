
#include "log.h"

int i = 512;
long l = 521L;
float f = 3.14f;
double d = 3.14L;
char *s = "World";


int func()
{
	FASTLOG(DEBUG, "Hello1: %d\n", i);
	FASTLOG(INFO, "Hello1: %d\n", i);
	FASTLOG ( ERROR, "Hello2: %ld\n", l);
}

int main()
{
	FASTLOG( INFO, "Hello3: %f\n", f);
	func();
	FASTLOG (INFO, "Hello4: %lf\n", d);
	FASTLOG( INFO, "Hello5: %s\n", s);
}
