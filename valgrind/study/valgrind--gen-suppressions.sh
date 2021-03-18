#!/bin/bash

# --leak-check=full 完全检查内存泄漏
# --show-reachable=yes 显示内存泄漏地点
# --trace-children=yes 跟入子进程
# --quiet 只打印错误信息
# --gen-suppressions=yes 生成抑制文件
function gen-suppressions() {
	if [ $# -lt 1 ]; then
		echo "usage: $0 [program]"
		exit
	fi

	valgrind --leak-check=full \
			 --show-reachable=yes \
			 --trace-children=yes \
			 --gen-suppressions=yes \
			 --quiet \
			 $*
}

# --tool=memcheck
# --quiet 只打印错误信息
function gen-suppressions2() {
	if [ $# -lt 1 ]; then
		echo "usage: $0 [program]"
		exit
	fi

#  --default-suppressions=no
#  --suppressions=/path/to/file.supp
	valgrind --tool=memcheck \
			 --gen-suppressions=yes \
			 --quiet \
			$*

# 然后调用
#callgrind_annotate --auto=yes \
#				callgrind.out.[pid]
}

gen-suppressions $*
