#!/bin/bash

rm -f *.out

for file in `ls test_*`
do 
	echo "Compile $file -> ${file%.*}.out ($*)"
	gcc $file -ltcmalloc_and_profiler \
		-pthread \
		-I./ -o ${file%.*}.out  -O0  $*
done



