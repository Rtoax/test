#!/bin/bash

gcc -Q -O3 --help=optimizers
# 下列选项控制优化:
#  -O<N>                       		
#  -Ofast                      		
#  -Og                         		
#  -Os                         		
#  -faggressive-loop-optimizations 	[启用]
#  -falign-functions           		[启用]
#  -falign-jumps               		[启用]
#	...

