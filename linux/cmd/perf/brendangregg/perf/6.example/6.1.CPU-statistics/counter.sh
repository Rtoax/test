#!/bin/bash

# Raw Counters
# The Intel 64 and IA-32 Architectures Software Developer's Manual Volume 3B: System Programming Guide, Part 2 and the BIOS and Kernel Developer's Guide (BKDG) For AMD Family 10h Processors are full of interesting counters, but most cannot be found in perf list. If you find one you want to instrument, you can specify it as a raw event with the format: rUUEE, where UU == umask, and EE == event number. Here's an example where I've added a couple of raw counters:
sudo perf stat -e cycles,instructions,r80a2,r2b1 ls
# If I did this right, then r80a2 has instrumented RESOURCE_STALLS.OTHER, and r2b1 has instrumented UOPS_DISPATCHED.CORE: the number of uops dispatched each cycle. It's easy to mess this up, and you'll want to double check that you are on the right page of the manual for your processor.
