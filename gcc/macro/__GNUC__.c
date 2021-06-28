#include <stdio.h>

#if defined (__GNUC__) \
		&& ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)	\
						|| __GNUC__ >= 5)
#endif

int main()
{
	printf("__GNUC__ = %d.\n", __GNUC__);
	printf("__GNUC_MINOR__ = %d.\n", __GNUC_MINOR__);
}
