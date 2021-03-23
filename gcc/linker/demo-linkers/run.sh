#!/bin/bash

rm -f *.o *.out

function test1() {
	# 单独编译main.c
	gcc -c main.c $*

	# 查看 main.o 中的字符
	echo -e ""
	nm -A main.o | sed 's/^/[nm] /g'

	# 反汇编 objdump
	echo -e ""
	objdump -S main.o | sed 's/^/[objdump] /g'

	# 两个 `callq` 操作。这两个 `callq` 操作包含了 `链接器存根`
	# `-r` 或 `--reloc ` 选项会打印文件的 `重定位` 条目
	echo -e "\n显示 链接器 存根"
	objdump -S -r main.o | sed 's/^/[objdump-r] /g'
}

function test2() {
	gcc main.c lib.c -o factorial
	objdump -S factorial | grep -e ".init" -e factorial -e main | sed 's/^/[objdump] /g'
	readelf -d factorial | grep \(INIT\) | sed 's/^/[readelf] /g'
	#objdump -S factorial | sed 's/^/[objdump] /g' | more
	#rm -f factorial
}

function test3() {
	gcc lib.c -c
	gcc main.c lib.o -o factorial
	./factorial 2>&1 | sed 's/^/[factorial] /g'
}

# 使用链接器
function test4() {
	gcc lib.c -c
	gcc main.c -c
	file main.o lib.o 2>&1 | sed 's/^/[file] /g'
	ld main.o lib.o -o factorial 2>&1 | sed 's/^/[ld] /g'
	gcc main.o lib.o -o factorial
}

function test5() {
	nasm -felf64 hello.asm && ld hello.o && ./a.out
}


rm -f *.o *.out
echo -e ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
test1
echo -e ""
echo -e ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
test2
echo -e ""
echo -e ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
test3
echo -e ""
echo -e ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
test4
test5
rm -f *.o *.out
