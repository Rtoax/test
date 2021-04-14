#!/bin/bash

# Frequency count tcp_sendmsg() size:
argdist -C 'p::tcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t size):u32:size'

# Summarize tcp_sendmsg() size as a power-of-2 histogram:
argdist -H 'p::tcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t size):u32:size'

# Count the libc write() call for PID 181 by file descriptor:
argdist -p 181 -C 'p:c:write(int fd):int:fd' 
