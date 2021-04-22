#!/bin/bash

# ustack(limit)
# ustack(mode[, limit])
# mode 参数：以不同形式输出调用张，目前支持两种 bpftrace 和 perf
sudo bpftrace -e 'k:do_nanosleep {printf("%s", ustack(perf));}'
