#!/bin/bash

sudo bpftrace -e 'profile:hz:49 /pid/ { @samples[ustack, kstack, comm] = count() }'
