#!/bin/bash

# 按进程名归纳所有 CPU 的交叉调用
dtrace -n 'sysinfo:::xcalls {@[execname] = count();}'

# 
