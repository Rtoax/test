#!/bin/bash

sudo perf sched record -- sleep 1

sudo perf sched script

rm -f perf.data*
