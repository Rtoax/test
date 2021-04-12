#!/bin/bash

sudo perf record -e block:block_rq_issue -ag
perf report
