#!/bin/bash

# L1 数据缓存未命中统计
sudo perf record -e L1-dcache-load-misses -c 100000 -a
sudo perf report -n --stdio
