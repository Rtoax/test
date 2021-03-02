#!/bin/bash

__bcc() {
	if [ $# -lt 1 ]; then
		echo $0 [bpf-prog.c]
		return 1
	fi
	prog=`basename $1`
	clang -O2 -emit-llvm -c $prog -o - | \
		llc -march=x86-64 -filetype=obj -o ${prog%.*}.o
}


alias bcc=__bcc
