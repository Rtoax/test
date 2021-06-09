#!/bin/bash
# 结合了  offcputime 和 wakeuptime 命令

# 跟踪 5s
offwaketime -Kp $(pgrep -n gzip) 5
