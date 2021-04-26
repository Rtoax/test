#!/bin/bash

# 首先 分析追踪点函数的参数列表
tplist -v syscalls:sys_enter_read
# syscalls:sys_enter_read
#  int nr;
#  unsigned int fd;
#  char * buf;
# size_t count;
argdist -H 't:syscalls:sys_enter_read():int:args->count'
argdist -H 't:syscalls:sys_exit_read():int:args->ret' # exit
# 使用 bpftrace 
bpftrace -e 't:syscalls:sys_enter_read{ @ = hist(args->count) }'

# Frequency count tcp_sendmsg() size:
argdist -C 'p::tcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t size):u32:size'

# Summarize tcp_sendmsg() size as a power-of-2 histogram:
argdist -H 'p::tcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t size):u32:size'

# Count the libc write() call for PID 181 by file descriptor:
argdist -p 181 -C 'p:c:write(int fd):int:fd' 
