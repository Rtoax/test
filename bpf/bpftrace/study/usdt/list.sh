#!/bin/bash
# 列出可执行二进制文件中的所有 usdt 点
# 以mysqld 为例
sudo bpftrace -l usdt:/usr/libexec/mysqld
