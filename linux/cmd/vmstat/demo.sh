#!/bin/bash

# 每隔一秒出书一次
vmstat 1

# 输出单位为 MB
vmstat 1 -Sm

# 输出非活动和活动缓存的明细
vmstat 1 -a

# free:	空闲内存
# buff:	存储IO缓冲区占用内存
# cache: 文件系统缓存占用的内存
# si:	页换入操作数量
# so:	换出
#procs -----------memory---------- ---swap-- -----io---- -system-- ------cpu-----
# r  b   swpd   free   buff  cache   si   so    bi    bo   in   cs us sy id wa st
# 1  0   3080 1692744 154368 5265800    0    0     4     8    9    4  0  0 100  0  0
