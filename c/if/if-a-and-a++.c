#include <stdio.h>

int IF(int i)
{
	printf("i = %d, !i = %d\n", i, !i);
	/* if 是从前往后比较 
	 * i=0;
	 * if( (++i) && (i) ) 为真*/
	if( (i) && (++i) )
	{
		printf("if is true.\n");
	}
	return 0;
}

int main () {
	IF(1);
	IF(0);
}
