kprobe机制用于在内核中动态添加一些探测点，可以满足一些调试需求。

kprobe API
-------------------------------------------
int register_kprobe(struct kprobe *p);//--------------------------注册kprobe探测点
void unregister_kprobe(struct kprobe *p);//-----------------------卸载kprobe探测点
int register_kprobes(struct kprobe **kps, int num);//-------------注册多个kprobe探测点
void unregister_kprobes(struct kprobe **kps, int num);//----------卸载多个kprobe探测点
int disable_kprobe(struct kprobe *kp);//--------------------------暂停指定定kprobe探测点
int enable_kprobe(struct kprobe *kp);//---------------------------回复指定kprobe探测点
void dump_kprobe(struct kprobe *kp);//----------------------------打印指定kprobe探测点的名称、地址、偏移

-------------------------------------------

kprobe trace events使用
kprobe事件相关的节点有如下：
-------------------------------------------
/sys/kernel/debug/tracing/kprobe_events-----------------------配置kprobe事件属性，增加事件之后会在kprobes下面生成对应目录。
/sys/kernel/debug/tracing/kprobe_profile----------------------kprobe事件统计属性文件。
/sys/kernel/debug/tracing/kprobes/<GRP>/<EVENT>/enabled-------使能kprobe事件
/sys/kernel/debug/tracing/kprobes/<GRP>/<EVENT>/filter--------过滤kprobe事件
/sys/kernel/debug/tracing/kprobes/<GRP>/<EVENT>/format--------查询kprobe事件显示格式




