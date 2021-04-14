#!/bin/bash

# Summarize the latency (time taken) by the vfs_read() function for PID 181:
funclatency -p 1 -u vfs_read

# Summarize the latency (time taken) by libc getaddrinfo(), as a power-of-2 histogram in microseconds:
funclatency -u 'c:getaddrinfo'
