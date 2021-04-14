#!/bin/bash

./opensnoop           # trace all open() syscalls
./opensnoop -T        # include timestamps
./opensnoop -U        # include UID
./opensnoop -x        # only show failed opens
./opensnoop -p 181    # only trace PID 181
./opensnoop -t 123    # only trace TID 123
./opensnoop -u 1000   # only trace UID 1000
./opensnoop -d 10     # trace for 10 seconds only
./opensnoop -n main   # only print process names containing "main"
./opensnoop -e        # show extended fields
./opensnoop -f O_WRONLY -f O_RDWR  # only print calls for writing
./opensnoop --cgroupmap mappath  # only trace cgroups in this BPF map
./opensnoop --mntnsmap mappath   # only trace mount namespaces in the map
