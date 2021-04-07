obj-m	:= pci_skel.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#KERNELDIR ?= /lib/modules/3.10.0-693.el7.x86_64/build
PWD       := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

