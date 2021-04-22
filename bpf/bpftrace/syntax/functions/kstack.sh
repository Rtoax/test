#!/bin/bash

# kstack(limit)
# kstack(modo[, limit])
sudo bpftrace -e 't:block:block_rq_insert {@[kstack(3), comm] = count();}'
