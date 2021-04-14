# test/linux

[https://github.com/Rtoax/test](https://github.com/Rtoax/test)

```
        _____________
       /\____    ____\
       \/___ \   \___/    _____     _____     ___  ___
            \ \   \      / ___ \   / ___ \   /\  \/  /
             \ \   \    /\ \_/\ \ /\ \_/\ \  \ \    /
              \ \   \   \ \ \\_\ \\ \ \\_\ \  \/    \
  R            \ \___\   \ \____ / \ \___  _\ /  /\  \
                \/___/    \/____/   \/___/\_//__/\_\__\
```

## 介绍

* `yaml`: 
* `acl`: Access Control Lists(/etc/fstab)
* `api`: 一些接口
* `barrier`: 内存屏障
* `block`: 块
* `bus`: bus总线
* `cgroup`: Control Groups(资源限制)
* `checkpoint`: 检测点
* `cma`: Contiguous Memory Allocation 连续内存分配
* `cmd`: 命令行 / ABI
* `config`: 配置项，如 CONFIG_HZ
* `cups`: Common UNIX Printing System 通用打印系统
* `dev`: 设备
* `devil`: 一种硬件IO接口语言
* `dingo`: a device driver architecture
* `dirtyCoW`: Linux内核写时复制的严重漏洞
* `dma`: 直接内存访问
* `driver`: 驱动
* `drm`: DRM 是Linux目前的主流图形显示框架
* `eBPF`: extend BPF
* `env`: 牛逼的环境变量，例如 LD_PRELOAD
* `essential`: 基础性内容
* `etc`: 配置文件夹
* `fs`: 文件系统
* `ftrace`: function tracer, ftrace 是个 framework，采用 plugin 的方式添加多种类 trace
* `functions`: 函数
* `housekeeping`: TODO
* `io`: IO
* `irq`: 中断
* `kernel`: 总结的内核文档
* `kma`: Kernel Memory Allocator (KMA)
* `kmalloc`: 基于slab的通用分配器
* `kni`: Kernel Network Interface
* `kprobe`: kprobe机制用于在内核中动态添加一些探测点
* `kvm`: 虚拟机
* `ldt-gdt`: 局部描述符表 和 全局描述符表
* `linux-bridge`: 一种虚拟交换机，网桥技术
* `LinuxKernelLearnCode`: LinuxKernelLearn
* `LinuxTestProject`: Linux Test Project https://github.com/linux-test-project/ltp
* `lru`: LRU（Least Recently Used, 最近最久未使用）算法
* `lsm`: 内核安全模块 Linux Security Modules
* `LTTng`: Linux Trace Toolkit next generation
* `macro`: 宏
* `mm`: 内存管理
* `module`: 模块
* `namespace`: 命名空间 （隔离资源，名称隔离）
* `NDL`: Like Devil
* `net`: 网络
* `netlink`: Netlink套接字是用以实现用户进程和内核进程通信的一种特殊的进程间通信
* `NUMA`: NUMA 系统是从SMP（Symmetric Multiple Processing，对称多处理器）系统演化而来
* `olpc`: OLPC 系统专为低端机型设计
* `pci`: PCI
* `proc`: /proc
* `process`: 进程： LWP(Lightweight Processes)，TSS(Task State Segment)
* `process-manage`: 进程管理
* `ptp`: PTP协议 （Precise Time Protocol 精准时间协议）
* `qemu`: QEMU虚拟机
* `rcu`: Read Copy Update
* `real-time`: 实时操作系统 RTOS
* `slab`: slab，slob，slub机制
* `slob`: 同上
* `slub`: 同上
* `SMP`: SMP-对称多处理 （Symmetrical Multi-Processing）
* `study`: 学习
* `syscalls`: 系统调用
* `timer`: 定时器 hrtimer等
* `timer_list`: 一个模块示例
* `vDSO`: Virtual Dynamically-lined Shared Object（为加快系统调用的速度）
* `vfio`: Virtual Function I/O（现代化的设备直通方案）
* `vfs`: 虚拟文件系统
* `watchdog`: 看门狗(/dev/watchdog;softlockup;hardlockup)
* `watchpoints`: 观测点




## 仓库

* 学习文档: [MoreThanDocs](https://github.com/Rtoax/MoreThanDocs)
* 代码学习: [test](https://github.com/Rtoax/test)


```
    wow   ▌▒█           ▄▀▒▌     
          ▌▒▒▀▄       ▄▀▒▒▒▐     
         ▐▄▀▒▒▀▀▀▀▄▄▄▀▒▒▒▒▒▐     
       ▄▄▀▒▒▒▒▒▒▒▒▒▒▒█▒▒▄█▒▐     
     ▄▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▄█▒▐ 
    ▐▒▒▒▄▄▄▒▒▒▒▒▒▒▒▒▒▒▒▒▀▄▒▒▌    
    ▌▒▒▐▄█▀▒▒▒▒▄▀█▄▒▒▒▒▒▒▒█▒▐    
   ▐▒▒▒▒▒▒▒▒▒▒▒▌██▀▒▒▒▒▒▒▒▒▀▄▌   
   ▌▒▀▄██▄▒▒▒▒▒▒▒▒▒▒▒░░░░▒▒▒▒▌   
   ▌▀▐▄█▄█▌▄▒▀▒▒▒▒▒▒░░░░░░▒▒▒▐   
  ▐▒▀▐▀▐▀▒▒▄▄▒▄▒▒▒▒▒▒▒▒▒▒▒▒▒▒▌  
  ▐▒▒▒▀▀▄▄▒▒▒▄▒▒▒▒▒▒░░░░░░▒▒▒▐   
   ▌▒▒▒▒▒▒▀▀▀▒▒▒▒▒▒▒▒░░░░▒▒▒▒▌   
   ▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▐    
    ▀▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▄▒▒▒▒▌    
      ▀▄▒▒▒▒▒▒▒▒▒▒▄▄▄▀▒▒▒▒▄▀     
     ▐▀▒▀▄▄▄▄▄▄▀▀▀▒▒▒▒▒▄▄▀       
    ▐▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▀▀         

```

