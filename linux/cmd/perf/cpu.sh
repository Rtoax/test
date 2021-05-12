#!/bin/bash

# CPU 时间如何消耗在用户空间和内核空间
echo "sleep 3"
sudo perf record -a -g -F 997 sleep 3
sudo perf report --stdio


