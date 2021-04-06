obj-m := memory.o
KERNEL_DIR=/lib/modules/`uname -r`/build
MODULE_DIR := $(shell pwd)

modules:
	make -C $(KERNEL_DIR) M=$(MODULE_DIR) modules
#	gcc nothing.c -o nothing.out
#

mknod:
	mknod /dev/memory c 60 0


clean:
	make -C $(KERNEL_DIR) M=$(MODULE_DIR) clean