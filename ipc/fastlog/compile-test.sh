#!/bin/bash

rm -f *.out

for file in `ls test-*`
do 
	echo "Compile $file -> ${file%.*}.out"
	gcc $file fastlog.c fastlog_parse.c fastlog_template.c common.c -pthread -I./ -o ${file%.*}.out -w $*
done



