#!/bin/bash

# 端口
ss -tulpn

# -i 显示内部信息 
# -e 显示扩展套接字信息
# -p 显示进程信息
# -m 内存用量
ss -tiepm
