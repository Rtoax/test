#!/bin/bash

SRCS=`ls demo-*.c`

for src in $SRCS
do
	exe=${src%.*}.out
	echo "Compile $src -> $exe"
	gcc common.c -pthread $src -o $exe
done
