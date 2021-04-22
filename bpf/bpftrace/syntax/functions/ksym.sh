#!/bin/bash

# ksym(addr)
# 将地址解析为函数名称
sudo bpftrace -e 'tracepoint:timer:hrtimer_start {@[args->function] = count()}'
#这将显示原始地址，使用ksym 显示函数名
sudo bpftrace -e 'tracepoint:timer:hrtimer_start {@[ksym(args->function)] = count()}'
