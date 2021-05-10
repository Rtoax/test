#!/bin/bash

# 统计 1 号进程的 系统调用
sudo strace -c -p 1

# 使用 dd 命令 拷贝 1K 数据 5w次
strace -c dd if=/dev/zero of=/dev/null bs=1k count=50k
