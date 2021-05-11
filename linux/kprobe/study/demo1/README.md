
Linux kprobe调试技术使用
* https://www.cnblogs.com/arnoldlu/p/9752061.html
* https://blog.csdn.net/luckyapple1028/article/details/52972315

-----------------------------------------

# kprobe API

```c
int register_kprobe(struct kprobe *p);//--------------------------注册kprobe探测点
void unregister_kprobe(struct kprobe *p);//-----------------------卸载kprobe探测点
int register_kprobes(struct kprobe **kps, int num);//-------------注册多个kprobe探测点
void unregister_kprobes(struct kprobe **kps, int num);//----------卸载多个kprobe探测点
int disable_kprobe(struct kprobe *kp);//--------------------------暂停指定定kprobe探测点
int enable_kprobe(struct kprobe *kp);//---------------------------回复指定kprobe探测点
void dump_kprobe(struct kprobe *kp);//----------------------------打印指定kprobe探测点的名称、地址、偏移
```

# kprobe 示例

该kprobe实例实现了do_fork的探测，该函数会在fork系统调用或者kernel_kthread创建内核线程时被调用。
对原%p修改为%pF后，可读性更强。可以显示函数名称以及偏移量。

可以通过`sudo cat /proc/kallsyms | grep do_fork`来验证地址和符号是否对应。
若没有sudo，看不到真实的地址。

