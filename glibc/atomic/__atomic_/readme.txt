

* `__ATOMIC_RELAXED`    //编译器和处理器可以对memory access做任何的reorder
* `__ATOMIC_ACQUIRE`    //保证本线程中,所有后续的读操作必须在本条原子操作完成后执行。
* `__ATOMIC_RELEASE`    //保证本线程中,所有之前的写操作完成后才能执行本条原子操作。
* `__ATOMIC_ACQ_REL`    //同时包含 memory_order_acquire 和 memory_order_release
* `__ATOMIC_CONSUME`    //释放消费顺序（release/consume）的规范正在修订中，而且暂时不鼓励使用 
* `__ATOMIC_SEQ_CST`    //最强的同步模式，同时也是默认的模型，具有强烈的happens-before语义
