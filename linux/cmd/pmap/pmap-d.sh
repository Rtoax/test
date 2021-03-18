#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage: $0 [PID]"
	exit 1
fi

# 查看进程地址
sudo pmap -d $1

# [rongtao@localhost pmap]$ sh pmap-d.sh 1
#1:   /usr/lib/systemd/systemd --switched-root --system --deserialize 21
#Address           Kbytes Mode  Offset           Device    Mapping
#000055f31b86a000    1416 r-x-- 0000000000000000 0fd:00000 systemd
#000055f31bbcc000     140 r---- 0000000000162000 0fd:00000 systemd
#000055f31bbef000       4 rw--- 0000000000185000 0fd:00000 systemd
