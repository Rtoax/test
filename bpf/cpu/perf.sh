#!/bin/bash

# 
perf stat -e mem_load_retired.l3_hit -e mem_load_retired.l3_miss -a -I 1000

# 定时采样
perf record -F 99 -a -g -- sleep 5
perf report -n --stdio
