#!/bin/bash

./tcpaccept           # trace all TCP accept()s
./tcpaccept -t        # include timestamps
./tcpaccept -P 80,81  # only trace port 80 and 81
./tcpaccept -p 181    # only trace PID 181
./tcpaccept --cgroupmap mappath  # only trace cgroups in this BPF map
./tcpaccept --mntnsmap mappath   # only trace mount namespaces in the map
