#!/bin/bash
# 
# Summarizing the tracepoint library names and numbers of tracepoints, on my Linux 4.10 system:
perf list | awk -F: '/Tracepoint event/ { lib[$1]++ } END {
    for (l in lib) { printf "  %-16.16s %d\n", l, lib[l] } }' | sort | column

# The following simple one-liner counts system calls for the executed command, and prints a summary (of non-zero counts):
sudo perf stat -e 'syscalls:sys_enter_*' date 2>&1 | awk '$1 != 0'


# Socket Buffers
# Tracing the consumption of socket buffers, and the stack traces, is one way to identify what is leading to socket or network I/O.
sudo perf record -e skb:consume_skb -ag
