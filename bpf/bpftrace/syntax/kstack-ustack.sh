#!/bin/bash

# 内置变量
# 多行字符串文本形式
sudo bpftrace -e 't:block:block_rq_insert {printf("<%s>Block IO by %s\n", comm, kstack);}'

# stack 可以作为映射表的键，以对出现次数进行统计
sudo bpftrace -e 't:block:block_rq_insert {@[kstack] = count();}'
