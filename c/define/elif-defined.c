#include <stdio.h>
#include <stdlib.h>

#define debug() printf("%s:%d \n", __func__, __LINE__)

#define A
//#define B

int main()
{
#if defined(A) 
    debug();
#elif defined(B)

#endif
    
    return 0;
}
