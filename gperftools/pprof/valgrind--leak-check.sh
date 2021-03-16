#!/bin/bash
# 荣涛2021年3月16日
# --leak-check=full 完全检查内存泄漏
# --show-reachable=yes 显示内存泄漏地点
# --trace-children=yes 跟入子进程
# --quiet 只打印错误信息

if [ $# -lt 1 ]; then
	echo "usage: $0 [program]"
	exit
fi

valgrind --leak-check=full \
		 --show-reachable=yes \
		 --trace-children=yes \
		 --quiet \
		 $*
