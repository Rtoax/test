#!/bin/bash

# --leak-check=full 完全检查内存泄漏
# --show-reachable=yes 显示内存泄漏地点
# --trace-children=yes 跟入子进程
# --quiet 只打印错误信息
# --gen-suppressions=yes 生成抑制文件
# --suppressions=file.txt 指定抑制文件
function suppressions() {
	if [ $# -lt 2 ]; then
		echo "usage: $0 [program] [suppressions file]"
		if [ ! -f $2 ]; then
			echo "File <$2> not exist"
		fi
		exit
	fi

	valgrind --leak-check=full \
			 --show-reachable=yes \
			 --trace-children=yes \
			 --suppressions=$2 \
			 --quiet \
			 $1
}

suppressions $*
