#!/bin/bash

# 查看 tp 点
bpftrace -l 'tracepoint:net:*'

# 
bpftrace -l 'tracepoint:tcp:*'
