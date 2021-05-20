#!/bin/bash

# -b 滚动输出，不清理屏幕
# -o 仅显示IO进程
# -d5 每5s输出一次
sudo iotop -bod5
