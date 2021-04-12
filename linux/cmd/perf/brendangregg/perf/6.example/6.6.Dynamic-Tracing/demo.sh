#!/bin/bash

# 添加
sudo perf probe --add tcp_sendmsg

# 统计
sudo perf record -e probe:tcp_sendmsg -a -g -- sleep 5

# 
sudo perf report --stdio

# 
sudo perf probe --del tcp_sendmsg
