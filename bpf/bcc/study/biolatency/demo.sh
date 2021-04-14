#!/bin/bash

./biolatency                    # summarize block I/O latency as a histogram
./biolatency 1 10               # print 1 second summaries, 10 times
./biolatency -mT 1              # 1s summaries, milliseconds, and timestamps
./biolatency -Q                 # include OS queued time in I/O time
./biolatency -D                 # show each disk device separately
./biolatency -F                 # show I/O flags separately
./biolatency -j                 # print a dictionary
