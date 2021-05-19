#!/bin/bash
# -c: 显示CPU报告
# -d: 显示磁盘报告
# -k: 使用KB代替512B
# -m: 使用MB代替
# -p: 包括单个分区的统计信息
# -t: 时间戳
# -x: 扩展统计信息
# -z: 不显示空活动汇总

# 子启动以来待选项 -c -d 汇总报告
iostat

# -m MB输出
iostat -m

# 较早期的版本的iostat输出默认使用块(512B),而非KB
POSIXLY_CORRECT=1 iostat

# 
iostat -xkdz

# 
iostat -p ALL
