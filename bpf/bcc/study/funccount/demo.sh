#!/bin/bash

# Count "tcp_send*" kernel function, print output every second:
funccount -i 1 'tcp_send*'

# Count "vfs_*" calls for PID 185:
funccount -p 185 'vfs_*'
