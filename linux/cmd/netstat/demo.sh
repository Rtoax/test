#!/bin/bash

# (default) 列出所有处于打开状态的套接字
# -a 列出所有套接字信息
# -s 网络软件栈信息
# -i 网络接口统计信息
# -r 列出路由表
# -p 进程信息
# -n 避免IP地址解析

# 
netstat -anp

# 
netstat -i
