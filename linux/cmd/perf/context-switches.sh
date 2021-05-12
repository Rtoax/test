#!/bin/bash

# 上下文切换事件
# 使用 perf list 查询
# context-switches OR cs 
sudo perf record -g -a -e context-switches sleep 2
sudo perf report --stdio
