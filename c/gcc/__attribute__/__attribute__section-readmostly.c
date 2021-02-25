#define __section(section)              __attribute__((__section__(section)))

static unsigned int __section(".read.mostly") a = 0xffffffff;
static unsigned int b __section(".read.mostly") = 0xffff0000;
static unsigned int c __section(".read.mostly2") = 0x00ffff00;
static unsigned int __section(".read.mostly2") d = 0xff0000ff;
