#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage: $0 [PID]"
	exit 1
fi

# 查看进程的扩展地址地址
sudo pmap -x $1

# [rongtao@localhost pmap]$ sh pmap-x.sh 1
#1:   /usr/lib/systemd/systemd --switched-root --system --deserialize 21
#Address           Kbytes     RSS   Dirty Mode  Mapping
#000055f31b86a000    1416     944       0 r-x-- systemd
#000055f31bbcc000     140     132     132 r---- systemd
#000055f31bbef000       4       4       4 rw--- systemd
