#!/bin/bash

# Distribution of read() bytes
sudo bpftrace -e 'tracepoint:syscalls:sys_exit_read /pid == 1/ { @bytes = hist(args->ret); }'
