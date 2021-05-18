#!/bin/bash

# 按应用程序名统计文件系统操作
/usr/sbin/dtrace -n fsinfo::: {@[execname] = count(); }
