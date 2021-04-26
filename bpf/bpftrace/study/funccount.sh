#!/bin/bash

# 
bpftrace -e 'k:tcp_* { @[probe] = count(); }'
