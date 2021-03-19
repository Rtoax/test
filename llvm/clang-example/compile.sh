#!/bin/bash
# 荣涛 日期：2021年3月19日
# https://llvm.org/docs/GettingStarted.html

# 编译生成 LLVM bitcode
echo -e "\n\n---------------------------------------------"
echo -e "编译生成 LLVM bitcode"
clang -O3 -emit-llvm hello.c -c -o hello.bc

# 执行 LLVM 
echo -e "\n\n---------------------------------------------"
echo -e "执行 LLVM bitcode 程序"
lli hello.bc

# 使用 llvm-dis 查看 bitcode 汇编码
echo -e "\n\n---------------------------------------------"
echo -e "使用 llvm-dis 查看 bitcode 汇编码"
llvm-dis < hello.bc | more 

# 用 LLC 原生汇编 bitcode
echo -e "\n\n---------------------------------------------"
echo -e "用 LLC 原生汇编 bitcode"
llc hello.bc -o hello.s

# 使用原生 gcc 编译 上面生成的 汇编
echo -e "\n\n---------------------------------------------"
echo -e "使用原生 gcc 编译 上面生成的 汇编"
gcc hello.s -o hello.out
./hello.out
