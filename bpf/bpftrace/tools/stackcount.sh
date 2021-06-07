#!/bin/bash

bpftrace --unsafe -e 'tracepoint:exceptions:page_fault_user /pid==1/ {
	@[kstack, ustack, comm] = count();} END { printf("stackcount >>>\n"); }'

