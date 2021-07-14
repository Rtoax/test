#!/bin/bash

for prog in $(ls *.c); 
do 
	echo "compile $prog"
	gcc $prog -o $prog.out -DHAVE_SYSTEMTAP_SYS_SDT_H
done
