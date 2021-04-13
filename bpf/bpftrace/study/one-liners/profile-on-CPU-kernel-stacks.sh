#!/bin/bash

# Profile on-CPU kernel stacks
sudo bpftrace -e 'profile:hz:99 { @[stack] = count(); }'
