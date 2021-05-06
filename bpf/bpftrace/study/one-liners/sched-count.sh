#!/bin/bash

# 统计进程级别的事件
sudo bpftrace -e 'tracepoint:sched:sched* { @[probe] = count(); } interval:s:5 { exit(); }'
