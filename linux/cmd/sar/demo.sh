#!/bin/bash

# sar: system activity information
# -B: 换页统计
# -H: 大页面统计信息
# -r: 内存使用率
# -R: 内存统计信息
# -S: 交换空间统计信息
# -W: 交换统计信息


# 报告所有 CPU 统计信息
sar -P ALL 1

# 文件熊他那个统计信息
sar -v 1
# dentunusd: 目录项缓存未用计数
# file-nr:	
# inode-nr:	
# pty-nr:	

# -d 磁盘统计信息
sar -d 1

# 页统计信息
sar -B 1

