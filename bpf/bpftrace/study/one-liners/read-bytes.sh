#!/bin/bash

# Kernel dynamic tracing of read() bytes
sudo bpftrace -e 'kretprobe:vfs_read { @bytes = lhist(retval, 0, 2000, 200); }'
