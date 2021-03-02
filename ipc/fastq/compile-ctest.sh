#!/bin/bash
# 荣涛 2021年1月27日

rm -f *.out

for file in `ls test-*`
do 
	echo "Compile $file -> ${file%.*}.out"
	gcc $file fastq.c -lcrypt -pthread -I./ -o ${file%.*}.epoll.stat.out -w $* -D_FASTQ_EPOLL=1 -D_FASTQ_STATS=1
	gcc $file fastq.c -lcrypt -pthread -I./ -o ${file%.*}.epoll.out -w $* -D_FASTQ_EPOLL=1 
	gcc $file fastq.c -lcrypt -pthread -I./ -o ${file%.*}.select.stat.out -w $* -D_FASTQ_SELECT=1  -D_FASTQ_STATS=1
	gcc $file fastq.c -lcrypt -pthread -I./ -o ${file%.*}.select.out -w $* -D_FASTQ_SELECT=1
done


