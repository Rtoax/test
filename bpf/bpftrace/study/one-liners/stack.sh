#!/bin/bash

function kernel_stack() {
sudo bpftrace -e 'profile:hz:99 { @[kstack] = count(); }'
}

kernel_stack
