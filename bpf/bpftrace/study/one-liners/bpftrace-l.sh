#!/bin/bash

sudo bpftrace -l 'tracepoint:syscalls:sys_enter_*' | sed 's/^/[bpftrace -l] /g'
