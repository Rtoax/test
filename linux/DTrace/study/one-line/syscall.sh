#!/bin/bash

# 打印出文件名和文件路径名
dtrace -n 'syscall::open:entry {printf("%s %s", execname, copyinstr(arg0));}'

