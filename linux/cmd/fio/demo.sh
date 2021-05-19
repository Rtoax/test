#!/bin/bash

# 随机读负载，其中IO大小为8KB，测试集大小为 5GB，非统一访问模式
fio --runtime=6 \
	--time_based \
	--clocksource=clock_gettime \
	--name=randread \
	--numjobs=1 \
	--rw=randread \
	--random_distribution=pareto:0.9 \
	--bs=8k \
	--size=5g \
	--filename=fio.tmp.out
