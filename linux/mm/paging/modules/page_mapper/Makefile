#name of compiled module
TARGET = page-mapper

#add all target objects
OBJS := chardev/chardev.o \
	page_mapper.o

#add all project source dirs
SUBDIRS = chardev \
	  utils

MDIR = drivers/misc

EXTRA_CFLAGS = -march=native -O2 -Wfatal-errors -Wall -Wno-declaration-after-statement
CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/kernel/$(MDIR)

ccflags-y := $(EXTRA_CFLAGS)

CHOST="x86_64-pc-linux-gnu"
CXXFLAGS="${CFLAGS}

obj-m += $(TARGET).o

page-mapper-objs := $(OBJS)

default:
	make -C $(KDIR) SUBDIRS=$(PWD) modules

md5:
	@echo $(shell md5sum -b page_mapper.ko > page_mapper.md5)

$(TARGET).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)

ifneq (,$(findstring 2.4.,$(CURRENT)))
install:
	su -c "cp -v $(TARGET).o $(DEST) && /sbin/depmod -a"
else
install:
	su -c "cp -v $(TARGET).ko $(DEST) && /sbin/depmod -a"
endif


clean:
	make -C $(KDIR) M=$(PWD) clean

	# Clean subdirs
	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; rm -f *.o *.ko .*.cmd .*.flags *.mod.c *.ko.cmd *.o.cmd); done
	# Clean in main dir
	rm -f *.o *.ko .*.cmd .*.flags *.mod.c
	# Remove temp folder
	rm -rf .tmp_versions
