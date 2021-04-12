#!/bin/bash

sudo perf sched record -- sleep 1

sudo perf sched replay -r -1

rm -f perf.data*
