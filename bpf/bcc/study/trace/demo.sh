#!/bin/bash


# Trace file names opened, using dynamic tracing of the kernel do_sys_open() function:
trace 'p::do_sys_open "%s", arg2'

# Same as before ("p:: is assumed if not specified):
trace 'do_sys_open "%s", arg2'

# Trace the return of the kernel do_sys_open() funciton, and print the retval:
trace 'r::do_sys_open "ret: %d", retval'

# Trace do_nanosleep() kernel function and the second argument (mode), with kernel stack traces:
trace -K 'do_nanosleep "mode: %d", arg2'

# Trace do_nanosleep() mode by providing the prototype (no debuginfo required):
trace 'do_nanosleep(struct hrtimer_sleeper *t, enum hrtimer_mode mode) "mode: %d", mode'

# Trace do_nanosleep() with the task address (may be NULL), noting the dereference:
trace 'do_nanosleep(struct hrtimer_sleeper *t, enum hrtimer_mode mode) "task: %x", t->task'

# Trace the libc library function nanosleep() and print the requested sleep details:
trace 'p:c:nanosleep(struct timespec *req) "%d sec %d nsec", req->tv_sec, req->tv_nsec'

# Trace the pthread_create USDT probe, and print arg1 as hex:
trace 'u:pthread:pthread_create "%x", arg1'

# 
./trace.py -U t:syscalls:sys_enter_brk
