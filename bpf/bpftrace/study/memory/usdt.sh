#!/bin/bash

LIBC=/usr/lib64/libc.so.6

# 查看 USDT 探针
sudo bpftrace -l usdt:$LIBC
