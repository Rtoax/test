#!/bin/bash

# 一些内置变量
sudo bpftrace -e 't:syscalls:sys_enter_open {
			printf("open by PID %d (%s), UID %d\n", pid, comm, uid);}'
# 跟踪上面的返回值
sudo bpftrace -e 't:syscalls:sys_exit_open {
			printf("open by (%s), returned %d\n", comm, args->ret);}'
