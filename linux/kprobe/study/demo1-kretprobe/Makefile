obj-m := kretprobe_example.o

CROSS_COMPILE=''

KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules 
clean:
	make -C $(KDIR) M=$(PWD) clean
#rm -rf *.ko *.o *.mod.o *.mod.c .*.cmd *.symvers  modul* .tmp_*
