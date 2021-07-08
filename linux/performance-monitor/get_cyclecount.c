#include <stdio.h>

static inline unsigned int get_cyclecount (void)
{
  unsigned int value;
  // Read CCNT Register
  asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value));  
  return value;
}


int main()
{
	unsigned int cyclecount = get_cyclecount();

	printf("cyclecount = %d\n", cyclecount);
}
