#!/bin/bash

./ext4slower             # trace operations slower than 10 ms (default)
./ext4slower 1           # trace operations slower than 1 ms
./ext4slower -j 1        # ... 1 ms, parsable output (csv)
./ext4slower 0           # trace all operations (warning: verbose)
./ext4slower -p 185      # trace PID 185 only
