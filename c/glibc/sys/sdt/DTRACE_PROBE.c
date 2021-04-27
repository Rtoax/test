#include <sys/sdt.h>

//tplist -l ./a.out
int main() {
	DTRACE_PROBE("hello-usdt", "probe-main");
}
