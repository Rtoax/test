#!/bin/bash

if [ $# -lt 1 ]; then
	echo "$0 [execution name]"
	exit 1
fi

./compile-test.sh

# minimal
# normal
# strict
# draconian
env HEAPCHECK=normal PPROF_PATH=/usr/local/bin/pprof ./$1
