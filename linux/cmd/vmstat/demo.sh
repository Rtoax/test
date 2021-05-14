#!/bin/bash

# 每隔一秒出书一次
vmstat 1

# 输出单位为 MB
vmstat 1 -Sm

# 输出非活动和活动缓存的明细
vmstat 1 -a
