#!/bin/bash

# 缺页 错误 火焰图
stackcount -f -PU t:exceptions:page_fault_user > out.pagefaults01.txt
flamegraph.pl --hash --width=800 --title="Page Fault Flame Graph" \
			--colors=java < out.pagefaults01.txt > out.pagefaults01.svg
