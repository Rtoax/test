#!/bin/bash

./execsnoop           # trace all exec() syscalls
./execsnoop -x        # include failed exec()s
./execsnoop -T        # include time (HH:MM:SS)
./execsnoop -U        # include UID
./execsnoop -u 1000   # only trace UID 1000
./execsnoop -u user   # get user UID and trace only them
./execsnoop -t        # include timestamps
./execsnoop -q        # add "quotemarks" around arguments
./execsnoop -n main   # only print command lines containing "main"
./execsnoop -l tpkg   # only print command where arguments contains "tpkg"
./execsnoop --cgroupmap mappath  # only trace cgroups in this BPF map
./execsnoop --mntnsmap mappath   # only trace mount namespaces in the map
