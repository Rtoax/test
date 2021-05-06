#!/bin/bash

sudo bpftrace -l 'tracepoint:syscalls:sys_enter_*'
