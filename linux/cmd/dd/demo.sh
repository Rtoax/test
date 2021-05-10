#!/bin/bash

# 执行 50w 次 1KB 的传输
dd if=/dev/zero of=/dev/null bs=1k count=500k
