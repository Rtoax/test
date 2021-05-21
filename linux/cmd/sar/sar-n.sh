#!/bin/bash

# -n DEV	网络接口统计信息
# -n EDEV	网络接口错误
# -n IP		IP数据报统计信息
# -n EIP	IP错误信息统计
# -n TCP	TCP统计信息
# -n ETCP	TCP错误统计信息
# -n SOCK	套集资使用

sudo sar -n SOCK | more

# 每秒 打印 TCP 统计信息
sudo sar -n TCP 1
