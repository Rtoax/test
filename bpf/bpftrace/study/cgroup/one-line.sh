#!/bin/bash

# 以99Hz 的频率鬼cgroup ID进行计数
bpftrace -e  'profile:hz:99 { @[cgroup] = count(); }'
