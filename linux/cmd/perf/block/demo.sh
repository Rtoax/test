#!/bin/bash

# 
perf list | grep block

# 用调用图来跟踪块设备问题
perf record -age block:block_rq_issue sleep 10
perf report | more 
