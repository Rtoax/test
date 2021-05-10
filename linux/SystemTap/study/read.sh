#!/bin/bash

stap -ve 'global stats; probe syscall.read.return { stats <<< $return; } 
	probe end {printf("\n\trval (byters)\n"); print(@hist_log(stats)); }'
