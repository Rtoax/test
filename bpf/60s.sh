#!/bin/bash
# 60秒分析

# 系统运行时间和平均负载
uptime

# 显示过去十条日志
dmesg | tail

# 虚拟内存，cpu时间
vmstat 1

# cpu详细时间
mpstat -P ALL 1

# 每个进程CPU使用情况
pidstat 1

# 存储设备IO指标
iostat -xz 1

# 内存使用情况
free -m

# -n：网络设备指标
sar -n DEV 1

# TCP
sar -n TCP,ETCP 1

# 
top
