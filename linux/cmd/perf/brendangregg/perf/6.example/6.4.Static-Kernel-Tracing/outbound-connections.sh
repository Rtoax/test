#!/bin/bash

# Outbound Connections
# There can be times when it's useful to double check what network connections are initiated by a server, from which processes, and why. You might be surprised. These connections can be important to understand, as they can be a source of latency.
# For this example, I have a completely idle ubuntu server, and while tracing I'll login to it using ssh. I'm going to trace outbound connections via the connect() syscall. Given that I'm performing an inbound connection over SSH, will there be any outbound connections at all?
sudo perf record -e syscalls:sys_enter_connect -a
