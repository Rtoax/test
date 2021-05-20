#!/bin/bash

# 启动内核块驱动跟踪机制获取跟踪裸数据，共 blkparse 处理以产生可读的输出
blktrace -d /dev/sr0 -o - | blkparse -i -

# 功能同上
btrace /dev/sr0
