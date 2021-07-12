#!/bin/bash

# ===================================
#  "fatal: The remote end hung up unexpectedly " 原因是推送的文件太大
# 那就简单了，要么是缓存不够，要么是网络不行，要么墙的原因
#修改提交缓存大小为500M，或者更大的数字
git config --global http.postBuffer 524288000

# some comments below report having to double the value:
git config --global http.postBuffer 1048576000

#配置git的最低速度和最低速度时间：
git config --global http.lowSpeedLimit 0
git config --global http.lowSpeedTime 999999  #单位 秒


