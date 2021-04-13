#!/bin/bash

# Block I/O tracing
sudo bpftrace -e 'tracepoint:block:block_rq_complete { @ = hist(args->nr_sector * 512); }'
