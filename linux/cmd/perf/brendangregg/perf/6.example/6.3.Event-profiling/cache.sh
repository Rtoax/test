#!/bin/bash

# For example, the following one-liner instruments Level 1 data cache load misses, 
# collecting a stack trace for one in every 10,000 occurrences:
sudo perf record -e L1-dcache-load-misses -c 10000 -ag -- sleep 5
