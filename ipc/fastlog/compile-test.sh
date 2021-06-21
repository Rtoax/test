#!/bin/bash

rm -f *.out

DEF="-DFASTLOG_HAVE_LIBXML2"

LIB="-lxml2 -pthread"

INC="-I./ -I./decoder/ -I/usr/include/libxml2"

SRC="fastlog.c fastlog_parse.c  fastlog_cycles.c fastlog_template.c fastlog_utils.c common.c"

for file in `ls test-*`
do 
	echo "Compile $file -> ${file%.*}.out"
	gcc $file  $SRC  \
			  $LIB $DEF $INC \
			  -o ${file%.*}.out -w -g -ggdb $*
done

SRC_DECODE="decoder/fastlog_decode_main.c \
			decoder/fastlog_decode_cli.c \
			decoder/fastlog_decode_list.c \
			decoder/fastlog_decode_reprintf.c \
			decoder/fastlog_decode_rb.c\
			decoder/fastlog_decode_output_txt.c\
			decoder/fastlog_decode_output_xml.c\
			decoder/fastlog_decode_file.c \
			decoder/linenoise/linenoise.c \
			decoder/bitmask/bitmask.c \
			decoder/hiredis/sds.c "

echo "Compile fastlog_decode_main.c -> decode.out"
gcc $SRC $SRC_DECODE \
	  $LIB  $DEF $INC \
	  -o decoder.out -w -g -ggdb $*

