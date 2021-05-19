#!/bin/bash

# 执行 50w 次 1KB 的传输
dd if=/dev/zero of=/dev/null bs=1k count=500k

# 读写测试
dd if=/dev/zero of=file1.out bs=1024k count=1k
dd if=file1.out of=/dev/null bs=1024k
