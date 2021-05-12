#!/bin/bash

# 调度器时延
# sched 命令记录并报告调度器统计信息
echo "sleep 3"
sudo perf sched record sleep 3

# 打印时延信息
sudo perf sched latency
# 输出如下
#  -----------------------------------------------------------------------------------------------------------------
#  Task                  |   Runtime ms  | Switches | Average delay ms | Maximum delay ms | Maximum delay at       |
#  -----------------------------------------------------------------------------------------------------------------
#     rngd:1519             |      0.151 ms |        3 | avg:    0.031 ms | max:    0.044 ms | max at: 3124036.411467 s
#	   systemd:(3)           |      0.770 ms |        4 | avg:    0.030 ms | max:    0.052 ms | max at: 3124037.816069 
