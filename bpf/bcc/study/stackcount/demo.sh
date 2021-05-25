#!/bin/bash

# Frequency count stack traces that lead to the submit_bio() function (disk I/O issue):
stackcount submit_bio

# Count stack traces that led to issuing block I/O, tracing its kernel tracepoint:
stackcount t:block:block_rq_insert

# 调用栈和次数
stackcount ktime_get
# 包含进程名字
stackcount -P  ktime_get

# 生成火焰图
stackcount -f -P -D 10 ktime_get  > out.stack.txt
wc out.stack.txt
flamegraph.pl --hash --bgcolors=grey < out.stack.txt > out.stack.svg

# 
stackcount -PU t:syscalls:sys_enter_brk

# 
