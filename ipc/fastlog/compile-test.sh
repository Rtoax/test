#!/bin/bash

rm -f *.out

SRC="fastlog.c fastlog_parse.c  fastlog_cycles.c fastlog_template.c fastlog_utils.c common.c"

for file in `ls test-*`
do 
	echo "Compile $file -> ${file%.*}.out"
	gcc $file  $SRC \
			  -pthread -I./ \
			  -o ${file%.*}.out -w -g -ggdb $*
done

SRC_DECODE="decoder/fastlog_decode_main.c \
			decoder/fastlog_decode_list.c \
			decoder/fastlog_decode_reprintf.c \
			decoder/fastlog_decode_rb.c\
			decoder/fastlog_decode_file.c "

echo "Compile fastlog_decode_main.c -> decode.out"
gcc $SRC $SRC_DECODE \
	  -pthread -I./ -I./decoder/ \
	  -o decoder.out -w -g -ggdb $*

