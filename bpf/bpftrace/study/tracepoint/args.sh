#!/bin/bash

# 查看 sys_enter_read 的参数列表
# 可以使用 args->fd, args->buf 方法使用
sudo bpftrace -lv tracepoint:syscalls:sys_enter_read
#tracepoint:syscalls:sys_enter_read 
#int nr; 
#unsigned int fd; 
#char demo.sh buf; 
#size_t count;

sudo bpftrace -e 'tracepoint:syscalls:sys_enter_clone {
				printf("-> clone() by %s PID %d\n", comm, pid);}
			tracepoint:syscalls:sys_exit_clone {
				printf("<- clone() return %d, %s PID %d\n", args->ret, comm, pid);}'
