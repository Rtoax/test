#!/bin/bash

if [ $# -lt 1 ]; then
	echo $0 [source.c]
	exit 1
fi

gcc -E $*
