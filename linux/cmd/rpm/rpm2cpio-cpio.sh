#!/bin/bash
# 提取 rpm 包 文件

file_rpm_pkg=$1

rpm2cpio $file_rpm_pkg | cpio -div
