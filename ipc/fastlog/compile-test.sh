#!/bin/bash

rm -f *.out

SRC="fastlog.c fastlog_parse.c  fastlog_cycles.c fastlog_template.c common.c"

for file in `ls test-*`
do 
	echo "Compile $file -> ${file%.*}.out"
	gcc $file  $SRC \
			  -pthread -I./ \
			  -o ${file%.*}.out -w -g -ggdb $*
done

echo "Compile fastlog_decode_main.c -> decode.out"
gcc fastlog_decode_main.c  $SRC \
			  -pthread -I./ \
			  -o decode.out -w -g -ggdb $*

