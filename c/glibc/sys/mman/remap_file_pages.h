#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <sys/mman.h>

//remap_file_pages - create a nonlinear file mapping
//remap_file_pages() 系统调用是用来建立一个非线性映射，也就是，
//在该文件的页被映射到存储器中的非连续的顺序映射. 
//mmap是将一个文件中的连续部分映射到虚拟内存中的一块连续的区域，它做的映射是线性映射，
//如果需要将一个文件中的不同部分以不同的顺序映射到虚拟内存中的连续区域，则需要使用多个
//mmap映射，从消耗的资源来看这样做代价比较昂贵（因为分配的vm_area_struct多了）。
//内核提供了一个系统调用叫做remap_file_pages，通过这个函数可以实现非线性映射，同时不需
//要分配更多的vm_area_struct。下面先看一下这个系统调用是怎么用的，然后说一下非线性映射
//在内核中是怎么实现的。

int remap_file_pages(void *addr, size_t size, int prot,
                    ssize_t pgoff, int flags);

