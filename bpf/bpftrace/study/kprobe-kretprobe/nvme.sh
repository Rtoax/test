#!/bin/bash

# 
bpftrace -e 'kprobe:nvme* { @[func]  =count(); }'
