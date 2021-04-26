#!/bin/bash

sudo bpftrace -e \
		'kprobe:cap_capable {
			time("%H:%M:%S  ");
			printf("%-6d %-6d %-16s %-4d %d\n", uid, pid, comm, arg2, arg3);
			//printf("%-6d %-6d %-16d\n", uid, pid, comm);
		}' \
		| grep -i capabilities
