#!/bin/bash

# 统计所有
sudo perf record -F 99 -a -g -- sleep 30

# 生成
sudo perf script > out.perf

# 
cd FlameGraph-1.0/
./stackcollapse-perf.pl ../out.perf > ../out.stacks

# 生成火焰图
./flamegraph.pl ../out.stacks > ../out.svg

