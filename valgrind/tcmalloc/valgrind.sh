#!/bin/bash
# 荣涛 2021年3月18日

LOG_FILE="valgrind.log"
LOG_SOCK="10.170.6.66:2049"
SUPPRESSION_FILE="./tcmalloc-suppression.txt"

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

	if [ ! -f $SUPPRESSION_FILE ]; then
		echo "File <$SUPPRESSION_FILE> not exist"
		exit
	fi
	valgrind --leak-check=full \
			 --show-reachable=yes \
			 --trace-children=yes \
			 --gen-suppressions=all \
			 --suppressions=$SUPPRESSION_FILE \
			 --quiet \
			 $*
}

# --leak-check=full 完全检查内存泄漏
# --show-reachable=yes 显示内存泄漏地点
# --trace-children=yes 跟入子进程
# --quiet 只打印错误信息
# --gen-suppressions=yes 生成抑制文件
# --suppressions=file.txt 指定抑制文件			 
# --time-stamp=yes 
# --quiet \

function suppressions() {
	if [ $# -lt 1 ]; then
		echo "usage: $0 [program]"
		exit
	fi
	if [ ! -f $SUPPRESSION_FILE ]; then
		echo "File <$SUPPRESSION_FILE> not exist"
		exit
	fi

	valgrind --leak-check=full \
			 --log-file=$LOG_FILE \
			 --log-socket=$LOG_SOCK \
			 --progress-interval=1 \
			 --child-silent-after-fork=yes \
			 --show-leak-kinds=all \
			 --show-reachable=yes \
			 --trace-children=no \
			 --allow-mismatched-debuginfo=no \
			 --suppressions=$SUPPRESSION_FILE \
			 --quiet \
			 $*
}

#gen-suppressions $*
suppressions $*

