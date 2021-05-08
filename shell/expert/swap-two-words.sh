#!/bin/bash

# 交换两个单词
echo This is me, one two three four | 
awk '{ 
	for (i=1;i<=NF;i++){ 
		printf "%s ", ($i=="one" ? "two" : $i=="two" ? "one" : $i); 
	}; 
	printf "\n"; 
}'
