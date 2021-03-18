#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage: $0 [PID]"
	exit 1
fi

# 查看进程使用内存情况
sudo pmap  $1

# [rongtao@localhost pmap]$ sh pmap.sh 1
#1:   /usr/lib/systemd/systemd --switched-root --system --deserialize 21
#000055f31b86a000   1416K r-x-- systemd
#000055f31bbcc000    140K r---- systemd
#000055f31bbef000      4K rw--- systemd

