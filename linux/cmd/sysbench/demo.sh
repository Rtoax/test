#!/bin/bash

# 执行 8 个线程
# 最多计算 100000 个质数
sudo sysbench --num-threads=8 --test=cpu --cpu-max-prime=100000 run
