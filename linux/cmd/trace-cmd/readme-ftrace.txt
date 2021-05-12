Ftrace

# 文件夹
------------------------
* test/linux/ftrace/
* test/linux/cmd/trace-cmd: trace-cmd->A front-end for Ftrace
* test/bpf/ftrace/


# 几个示例
--------------------------
首先需要打开tracing功能
```bash
echo 1 > /sys/kernel/debug/tracing/tracing_on
```

```bash
cd /sys/kernel/debug/tracing
echo function > current_tracer
cat trace
```

```bash
echo function_graph > current_tracer
cat trace
```

```bash
echo "*sched*" > set_ftrace_filter
echo function > current_tracer
cat trace
```

```bash
echo yield > set_ftrace_filter 
echo schedule >> set_ftrace_filter
```


# 原理
-----------------------------
●gcc’s profiler option: -pg
●Adds special mcount function call
  ●all functions call mcount
  ●mcount is a trampoline(蹦床)
