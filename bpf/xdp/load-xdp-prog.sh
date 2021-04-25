#!/bin/bash

# ip 命令充当 XDP前端 的能力
ip link set dev eth0 xdp obj program.o sec mysection
