#!/bin/bash

stackcount -U t:exceptions:page_fault_user
stackcount t:exceptions:page_fault_kernel

# 利用 -P 参数跟踪某个进程的结果
stackcount -PU t:exceptions:page_fault_user
