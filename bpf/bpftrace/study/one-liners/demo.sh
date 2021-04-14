#!/bin/bash

#1. Listing probes
bpftrace -l 'tracepoint:syscalls:sys_enter_*'

#2. Hello world
bpftrace -e 'BEGIN { printf("hello world\n"); }'

#3. File opens
bpftrace -e 'tracepoint:syscalls:sys_enter_open { printf("%s %s\n", comm, str(args->filename)); }'

#4. Syscall counts by process
bpftrace -e 'tracepoint:raw_syscalls:sys_enter { @[comm] = count(); }'

#5. Distribution of read() bytes
bpftrace -e 'tracepoint:syscalls:sys_exit_read /pid == 18644/ { @bytes = hist(args->retval); }'

#6. Kernel dynamic tracing of read() bytes
bpftrace -e 'kretprobe:vfs_read { @bytes = lhist(retval, 0, 2000, 200); }'

#7. Timing read()s
bpftrace -e 'kprobe:vfs_read { @start[tid] = nsecs; }
    kretprobe:vfs_read /@start[tid]/ { @ns[comm] = hist(nsecs - @start[tid]); delete(@start[tid]); }'

#8. Count process-level events
bpftrace -e 'tracepoint:sched:sched* { @[name] = count(); } interval:s:5 { exit(); }'

#9. Profile on-CPU kernel stacks
bpftrace -e 'profile:hz:99 { @[stack] = count(); }'

#10. Scheduler tracing
bpftrace -e 'tracepoint:sched:sched_switch { @[stack] = count(); }'

#11. Block I/O tracing
bpftrace -e 'tracepoint:block:block_rq_complete { @ = hist(args->nr_sector * 512); }'


