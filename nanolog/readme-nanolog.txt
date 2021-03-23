Nanolog

使用的技术
-----------------------------------
1. 无锁编程(不用CAS, 使用内存屏障);
2. C++11 __thread 关键字, 每个线程一个独立的 无锁环形队列;
3. 减少 CacheMiss: 数据结构, 局部性原理;
4. 避免函数调用: gettimeofday() -> tsc 寄存器;
5. 对信息进行压缩;

链接
-----------------------------------
* [源码](https://github.com/PlatformLab/NanoLog);
* [论文 NanoLog: A Nanosecond Scale Logging System](https://link.zhihu.com/?target=https%3A//www.usenix.org/conference/atc18/presentation/yang-stephen)

-----------------------------------
* test/nanolog
* 

