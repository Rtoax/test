#!/bin/bash

./runqlat            # summarize run queue latency as a histogram
./runqlat 1 10       # print 1 second summaries, 10 times
./runqlat -mT 1      # 1s summaries, milliseconds, and timestamps
./runqlat -P         # show each PID separately
./runqlat -p 185     # trace PID 185 only
