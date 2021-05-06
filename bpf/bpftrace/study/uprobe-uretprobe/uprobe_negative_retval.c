#include <unistd.h>

//sudo bpftrace -e 'uprobe:./a.out:function1 { printf("%s\n", usym(reg("ip"))); }'
__attribute__ ((noinline)) int function1(int x)
{
  return x;
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused))) {
  for (int x = -120; x <= 100; x++) {
    function1(x);
  }
  return -100;
}
