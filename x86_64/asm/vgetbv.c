#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define VGA_amd64 1

/**
 * Get Value of Extended Control Register
 *
 *	XGETBV指令是在任何访问级别均可调用的指令，即在Ring3的应用程序层也可使用XGETBV指令。 
 *	虽然应用程序层可以使用XGETBV指令，但在实际使用时会遇到问题。
 */
static bool have_xgetbv ( void )
{
#if defined(VGA_amd64)
   unsigned long long int w;
   __asm__ __volatile__("movq $0,%%rcx ; "
                        ".byte 0x0F,0x01,0xD0 ; " /* xgetbv */
                        "movq %%rax,%0"
                        :/*OUT*/"=r"(w) :/*IN*/
                        :/*TRASH*/"rdx","rcx");
   if ((w & 6) == 6) {
      /* OS has enabled both XMM and YMM state support */
      return true;
   } else {
      return false;
   }
#else
   return false;
#endif
}

int main() {

	printf("%d\n", have_xgetbv());
	//printf("%d\n", _xgetbv());
	return 0;
}
