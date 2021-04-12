#!/bin/bash

# Sampling CPU stacks at 
# 1. 99 Hertz (-F 99), 
# 2. for the entire system (-a, for all CPUs), 
# 3. with stack traces (-g, for call graphs), 
# 4. for 10 seconds:
sudo perf record -F 99 -a -g -- sleep 10
sudo perf report --stdio
