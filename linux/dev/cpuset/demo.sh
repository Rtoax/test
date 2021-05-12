#!/bin/bash

# 独占 CPU 组，不允许其他进程使用
# 具体查看 man cpuset(7)
sudo mkdir /dev/cpuset	# 
cd /dev/cpuset
mkdir prodset			# 创建一个名为 prodset 的CPUset
cd prodset			
echo 7-10 > cpus		# 分配 7-10 CPUs
echo 1 > cpu_exclusize	# 让 prodset 独占
echo 1159 > tasks		# 分配 PID 1159 给 prodset
