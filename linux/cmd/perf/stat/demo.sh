#!/bin/bash

# 跟踪，统计系统中的ext4 调用
sudo perf stat -e "ext4:*" -a
