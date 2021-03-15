#!/bin/bash

rm -f *.out

for file in `ls *.c`
do 
	echo "Compile $file -> ${file%.*}.out"
	gcc $file -pthread -I./ -o ${file%.*}.out -w $*
done



