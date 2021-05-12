#!/bin/bash


taskset 0x0001 iperf3 --bind 127.0.0.1 -s

taskset 0x0002 iperf3  --bind 127.0.0.1 -c 127.0.0.1
