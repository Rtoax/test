#!/bin/bash
# rongtao 日期

# 普通
gcc -Wall program.c -o sum.out

# verbose mode 冗长模式
# `cc1` 编译器将编译我们的 `C` 代码并且生成 `/tmp/ccvUWZkF.s` 汇编文件
# 汇编文件被 `GNU as` 编译器编译为目标文件
# 最后我们的目标文件会被 `collect2` 链接到一起
gcc -v -ggdb program.c -o sum.out 2>&1  | sed 's/^/[Verbose] /g'

ldd sum.out | sed 's/^/[ldd] /g'

# -nostdlib 报错
gcc -nostdlib program.c -o sum.out 2>&1  | sed 's/^/[-nostdlib] /g'

# 即使链接标准库，还是编译不行
gcc -nostdlib -lc -ggdb program.c -o sum.out 2>&1| sed 's/^/[-nostdlib-lc] /g'

# _start 在文件 /lib64/crt1.o 中
objdump -d /lib/../lib64/crt1.o 2>&1| sed 's/^/[crt1.o] /g'

# 不会段错误, 但是会段错误
gcc -nostdlib /lib64/crt1.o /lib64/crti.o -lc -ggdb program.c
# ./a.out 段错误

# 
objdump -D /lib64/crti.o 2>&1| sed 's/^/[crti.o] /g'

# 成功编译，成功运行
gcc -nostdlib /lib64/crt1.o /lib64/crti.o /lib64/crtn.o -lc -ggdb program.c 2>&1| sed 's/^/[crtn.o] /g'
./a.out

# `_start` 总是被默认的 `ld` 脚本链接到程序 `.text` 段的起始位置
ld --verbose | grep ENTRY 2>&1| sed 's/^/[ld--verbose] /g'
