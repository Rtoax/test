obj-m:=MyCharDevice.o
PWD:=$(shell pwd)
CUR_PATH:=$(shell uname -r)
KERNEL_PATH:=/usr/src/kernels/$(CUR_PATH)

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules
	
clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean
	
