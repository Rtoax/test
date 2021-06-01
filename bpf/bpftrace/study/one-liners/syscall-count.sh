#!/bin/bash

# Syscall counts by process
sudo bpftrace -e 'tracepoint:raw_syscalls:sys_enter { @[comm] = count(); }'

# 应用程序正在使用poll 系统调用占用的文件描述符数量
bpftrace -e 'tracepoint:syscalls:sys_enter_poll { @[comm, args->nfds] = count(); }'
