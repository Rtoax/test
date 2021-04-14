#!/bin/bash

./profile             # profile stack traces at 49 Hertz until Ctrl-C
./profile -F 99       # profile stack traces at 99 Hertz
./profile -c 1000000  # profile stack traces every 1 in a million events
./profile 5           # profile at 49 Hertz for 5 seconds only
./profile -f 5        # output in folded format for flame graphs
./profile -p 185      # only profile process with PID 185
./profile -L 185      # only profile thread with TID 185
./profile -U          # only show user space stacks (no kernel)
./profile -K          # only show kernel space stacks (no user)
./profile --cgroupmap mappath  # only trace cgroups in this BPF map
./profile --mntnsmap mappath   # only trace mount namespaces in the map
