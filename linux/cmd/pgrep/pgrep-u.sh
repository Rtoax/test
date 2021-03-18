#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage: $0 [user name]"
	exit 1
fi	

pgrep -u $1
