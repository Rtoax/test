#!/bin/bash

# 进程剖析
# perf record -g command
perf record -g ls

perf report --stdio
