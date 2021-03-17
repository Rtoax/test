#!/bin/bash
# 如果我们的程序集成了第三方模块，但又不希望检查他们的内存泄漏问题，可以通过参数指定valgrind忽略他们。
# suppressions 参数
# suppressions 参数告诉valgrind忽略指定的错误，用法如下

valgrind --leak-check=full \
		 --error-exitcode=1 \
		 --suppressions=../test/valgrd_ignore.txt  \
		 $*
#TODO 2021年3月17日

