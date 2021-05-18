/**
 *	BTS 指令: bit test and set
 *	
 *	modify: RToax
 *	https://www.felixcloutier.com/x86/bts
 *	https://stackoverflow.com/questions/1983303/using-bts-assembly-instruction-with-gcc-compiler
 */

#include <stdio.h>
#include <stdbool.h>

typedef unsigned long LongWord;

#if ORGINAL
#define DivLongWord(w) ((unsigned)w >> 5)
#define ModLongWord(w) ((unsigned)w & (32-1))

inline void SetBit(LongWord array[], const int bit)
{
   array[DivLongWord(bit)] |= 1 << ModLongWord(bit);
}

inline bool TestBit(const LongWord array[], const int bit)
{
    return (array[DivLongWord(bit)] & (1 << ModLongWord(bit))) != 0;
}
#elif defined(BTS1)
//The following assembler code works, but is not optimal, 
//as the compiler can't optimize register allocation:
inline void SetBit(LongWord* array, const int bit)
{
	asm("bts %1,%0" : "+m" (*array) : "r" (bit));
}

#else
# error Must #define ORGINAL|BTS1
#endif

int main() 
{
	unsigned long l = 0;
	SetBit(&l, 0);

	printf("0x%lx\n", l);
}
