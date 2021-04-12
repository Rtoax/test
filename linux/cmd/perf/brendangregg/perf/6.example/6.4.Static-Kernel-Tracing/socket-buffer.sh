#!/bin/bash

# Socket Buffers
# Tracing the consumption of socket buffers, and the stack traces, is one way to identify what is leading to socket or network I/O.
sudo perf record -e skb:consume_skb -ag
