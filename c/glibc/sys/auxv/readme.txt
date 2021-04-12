一种机制：当一个程序运行，从内存态将 ELF 特定的信息加载至 用户态

```bash
$ od -t d8 /proc/$$/auxv
```

AT_SYSINFO_EHDR: 0x7fff35d0d000
AT_HWCAP:        bfebfbff
AT_PAGESZ:       4096
AT_CLKTCK:       100
AT_PHDR:         0x400040
AT_PHENT:        56
AT_PHNUM:        9
AT_BASE:         0x0
AT_FLAGS:        0x0
AT_ENTRY:        0x40164c
AT_UID:          1000
AT_EUID:         1000
AT_GID:          1000
AT_EGID:         1000
AT_SECURE:       0
AT_RANDOM:       0x7fff35c2a209
AT_EXECFN:       /usr/bin/sleep
AT_PLATFORM:     x86_64

----------------------------------------------
用户空间应用程序与内核之间有许多交流机制。系统调用和伪文件系统（诸如：/proc和/sys）广为人知。信号也同样广为人知；内核利用信号通知进程的各种同步或异步事件——例子：当进程尝试写一个破碎的管道或子进程终止时。

内核和用户空间交流还有许多复杂的机制。包括Linux专用的netlink sockets和user-mode helper功能。Netlink套接字为内核交换信息提供了一套套接字风格的API。user-mode helper功能允许内核自动调用用户空间的可执行文件；这个机制被用于许多地方，包括控制组的实现和piping core dumps to a user-space application。

辅助向量(auxiliary vector)，一个从内核到用户空间的信息交流机制，它一直保持透明。然而，在GNU C库(glibc)2.16发布版中添加了一个新的库函数”getauxval()”，这似乎在六月底，现在它已经变得更加可见。历史上，许多UNIX系统实现过辅助向量功能。本质上，它是一个键值对列表，当一个新的可执行映像被加载到进程中时被内核的ELF二进制加载器（fs/binfmt_elf.c文件，在内核源码中）构造。这个列表被放在进程地址空间的特定位置；在Linux系统上它存在于用户地址空间的高位(high end)，在栈，命令行参数（argv）和环境变量（environ）的正上方（向下生长）。
