#!/bin/bash

./tcpconnect           # trace all TCP connect()s
./tcpconnect -t        # include timestamps
./tcpconnect -d        # include DNS queries associated with connects
./tcpconnect -p 181    # only trace PID 181
./tcpconnect -P 80     # only trace port 80
./tcpconnect -P 80,81  # only trace port 80 and 81
./tcpconnect -U        # include UID
./tcpconnect -u 1000   # only trace UID 1000
./tcpconnect -c        # count connects per src ip and dest ip/port
./tcpconnect -L        # include LPORT while printing outputs
./tcpconnect --cgroupmap mappath  # only trace cgroups in this BPF map
./tcpconnect --mntnsmap mappath   # only trace mount namespaces in the map

