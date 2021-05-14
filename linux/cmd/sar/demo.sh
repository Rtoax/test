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
