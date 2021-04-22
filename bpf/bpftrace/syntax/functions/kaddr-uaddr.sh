#!/bin/bash

# kaddr(char *name)
# uaddr(char *name)
# 
# bash 函数被调用时，开始查找用户空间 ps1_prompt 变量
#	然后解析地址用字符串形式答应出来
sudo bpftrace -e 'uprobe:/bin/bash:readline {
		printf("PS1: %s\n", str(*uaddr("ps1_prompt")));}'
