#!/bin/bash

# 显示磁盘上缓存命中和未命中之间的巨大差异
# -T 测试带缓存的读
# -t 测试磁盘设备读
hdparm -Tt /dev/vda
