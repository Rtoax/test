# page_mapper
Linux kernel module to map memory to the userspace via mmap

HowTo Build: 

    make clean
    
    make

Load module:   load_module.sh or insmod page-mapper.ko

Unload module: unload_module.sh or rmmod page-mapper.ko

Tested on OpenSuse Leap 43.2 kernel 4.10.0

After loading the module, you can open and mmap memory through /dev/mem2 device file.
