#!/bin/bash

# 列出库中的 USDT 追踪点
tplist -l /lib/libc.so.6

# 跟踪点参数名字
tplist -l /lib/libc.so.6 -v libc:setjmp

tplist -v syscalls:sys_enter_read
