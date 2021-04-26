#!/bin/bash

bpftrace -e 'u:/lib64/libpthread.so:pthread_create {
	printf("%s by %s (%d)\n", probe, comm, pid);}'
