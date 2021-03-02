#obj-m += hello-module1.o
#obj-m += hello-module2.o
#obj-m += hello-module3.o
#obj-m += hello-module5.o
obj-m += moduleTimer.o

objs = $(wildcard *.o)
kobjs = $(wildcard *.ko)
modc = $(wildcard *.mod.c)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	#make -C /lib/modules/2.6.32-35-generic/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	#make -C /lib/modules/2.6.32-35-generic/build M=$(PWD) modules
	#echo $(kobjs)
	#echo $(objs)
	-rm $(kobjs) 
	-rm $(modc) 
	-rm $(objs) 
