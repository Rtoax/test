# 内存申请释放策略

## 多线程使用推荐

在多线程环境中，tcmalloc和jemalloc效果明显，
当线程数固定，不会频繁创建退出时候，可以使用jemalloc
			反之，使用tcmalloc可能是最好的选择。

## 本项目中的目录地址

----------------------------------------
*** test/jemalloc/	FaceBook开源
*** test/tcmalloc/	Google开源
*** test/ptmalloc/	效率比较低(glibc标配)
*** test/mimalloc/	MicroSoft开源
*** test/glibc/malloc	GNU library(ptmalloc)
*** test/c/glibc/dlfcn/malloc 内存接口动态注入

## 不同策略提供的内存管理接口

### ptmalloc (glibc)
--------------------------------------------
*** test/c/glibc/mcheck
*** test/c/glibc/malloc

### tcmalloc (Google)
--------------------------------------------


### jemalloc (FackBook)
--------------------------------------------
















