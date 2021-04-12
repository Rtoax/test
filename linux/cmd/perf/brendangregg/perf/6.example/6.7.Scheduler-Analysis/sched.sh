#!/bin/bash

sudo perf sched record -- sleep 1
sudo perf sched latency
sudo perf sched map
sudo perf sched timehist
sudo perf sched timehist -MVw
sudo perf report --stdio
sudo perf script --header

rm -f perf.data*
