#!/bin/bash

# 
sudo bpftrace -e 'uprobe:bash:readline { printf("%s\n", ustack(perf)); }'
sudo bpftrace -e 'uprobe:bash:readline { printf("%s\n", ustack(perf, 3)); }'
