#!/bin/bash

# 位置参数
echo $0 [pid]
sudo bpftrace watchconn.bt $*

# 或者可以直接
sudo bpftrace -e 'BEGIN {printf("Hello, %s\n", str($1)); exit()}' RToax
