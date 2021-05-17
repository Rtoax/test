大页内存

----------------------------------
*** test/c/glibc/hugetlbfs	如何使用大页内存 hugetlbfs.h
*** test/hugepage/alloc/hugepage_manage	一个使用大页内存的源代码示例
*** test/dpdk/hugepage	DPDK的大页内存机制
*** test/linux-5.6.5/tools/testing/selftests/vm  linux-5.6.5内核自测测试例


挂载大页面
----------------------------------
mkdir /mnt/hugetlbfs
mount -t hugetlbfs none /mnt/hugetlbfs -o pagesize=2048K
