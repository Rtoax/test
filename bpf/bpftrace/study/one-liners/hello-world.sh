#!/bin/bash

# Hello
sudo bpftrace -e 'BEGIN { printf("Hello BPF!\n"); exit(); }'
