#!/bin/bash

# 跟踪 ssh 登录的进程执行过程
sudo dtrace -n 'exec-success {printf("%d %s", timestamp, curpsinfo->pr_psargs)}'
