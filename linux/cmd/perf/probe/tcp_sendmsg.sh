#!/bin/bash

# 跟踪 probe:tcp_sendmsg 5s
sudo perf probe --add=tcp_sendmsg
sudo perf record -e probe:tcp_sendmsg -aR sleep 5
sudo perf report --stdio
sudo perf probe --del=tcp_sendmsg
