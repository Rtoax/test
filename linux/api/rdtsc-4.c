#include <stdio.h>
#include <stdint.h>

#if defined __x86_64__
static inline uint64_t __tsc(void)
{
   unsigned int a, d;
   asm volatile ("rdtsc" : "=a" (a), "=d"(d));
   return ((unsigned long) a) | (((unsigned long) d) << 32);
}

#else
static inline uint64_t __tsc(void)
{
   uint64_t c;
   asm volatile ("rdtsc" : "=A" (c));
   return c;
}

#endif

int main() {
	uint64_t v1 = 0, v2 = 0;
	printf("%lx - %lx\n", (0xffffULL << 48), ~(0xffffULL << 48));
	while(1) {
		v1 = __tsc() & ~(0xffffULL << 48);
		sleep(1);
		v2 = __tsc() & ~(0xffffULL << 48);
		printf("%lx - %lx = %lx\n", v2, v1, v2 - v1);
	}
}
