#!/bin/bash

# 测量 ext4 文件系统读操作的时间
sudo strace -ttT -p 1

# 
sudo strace cksum -tttT /usr/bin/cksum
