#!/bin/bash

# Scheduler tracing
sudo bpftrace -e 'tracepoint:sched:sched_switch { @[stack] = count(); }'
