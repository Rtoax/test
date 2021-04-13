#!/bin/bash

# Count process-level events
sudo bpftrace -e 'tracepoint:sched:sched* { @[name] = count(); } interval:s:5 { exit(); }'
