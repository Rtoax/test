#!/bin/bash

# 以直方图形式显示vfs_read 的返回值(读取字节数或者错误码)
bpftrace -e 'kretprobe:vfs_read { $bytes = hist(retval); }'
