PWD := $(shell pwd)
USDT_DEVICE := /dev/bpf_usdt
MAJOR = $(shell cat /proc/devices | grep bpf | awk '{ printf($$1) }')

all:
	$(MAKE) -C $(PWD)/src

bin:

bin-register:
	gcc tests/register.c -o register

bin-reader:
	gcc tests/reader.c -o reader

bin-dyn:
	gcc -c tests/dyn.c -o build/dyn.o
	gcc -o build/dyn-x86_64.o -c tests/dyn-x86_64.s
	gcc build/dyn-x86_64.o build/dyn.o -o dyn

insmod:
	@sudo insmod src/bpf_usdt_driver.ko

load: insmod
	sudo mknod $(USDT_DEVICE) c $(MAJOR) 0

rmmod:
	sudo rmmod bpf_usdt_driver.ko

unload: rmmod
	sudo rm /dev/bpf_usdt

clean:
	$(MAKE) -C $(PWD)/src clean
	rm -f build/*.o dyn register  reader