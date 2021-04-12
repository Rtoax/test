#!/bin/bash

sudo perf list | grep L1-dcache

sudo perf stat -e L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores ls
