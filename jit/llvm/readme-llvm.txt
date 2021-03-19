Low Level Virtual Machine - LLVM

底层的虚拟机；编译器；

起初想做一个底层的类似于 JVM 一样的虚拟机，但是现在他和虚拟机没有任何关系.
提供一套中立的中间代码 和 编译基础设施，并围绕这些设施提供了一套全新的编译
策略，使得优化能够在 编译、链接、运行环境执行过程中，以及安装以后有效的方式
进行。

--------------------------------------------------------------------------
1. test/llvm
2. test/compile/llvm
3. test/jit/llvm


我的系统中关于 LLVM 的头文件如下：
--------------------------------------------------------------------------
[rongtao@localhost include]$ pwd
/usr/include
[rongtao@localhost include]$ find -name "*llvm*"
./boost159/boost/predef/compiler/llvm.h
./mesa-private/llvm
./mesa-private/llvm/Config/llvm-config-64.h
./mesa-private/llvm/Config/llvm-config.h
./mesa-private/llvm-c
