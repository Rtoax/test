#!/bin/bash

# 用户态程序
sudo bpftrace -e 'uprobe:/bin/bash:readline { printf("%s-> %s\n", comm, usym(reg("ip"))); }'

# 获取bash readline 的提示符
sudo bpftrace -e 'uprobe:/bin/bash:readline { printf("PS1: %s\n", str(*uaddr("ps1_prompt"))); }'

