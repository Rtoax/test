#!/bin/bash
# perf 性能远高于 strace

dd if=/dev/zero of=/dev/null bs=512 count=10000k
#5242880000 bytes (5.2 GB) copied, 3.53031 s, 1.5 GB/s

perf stat -e 'syscalls:sys_enter_*' dd if=/dev/zero of=/dev/null bs=512 count=10000k | awk '$1 != 0'
#5242880000 bytes (5.2 GB) copied, 9.14225 s, 573 MB/s

strace -c dd if=/dev/zero of=/dev/null bs=512 count=10000k
#5242880000 bytes (5.2 GB) copied, 218.915 s, 23.9 MB/s
