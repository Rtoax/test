#!/bin/bash

echo "展示 block:block_rq_issue 的调用参数"
bpftrace -lv tracepoint:block:block_rq_issue

echo "IO请求的尺寸是多少"
bpftrace -e 'tracepoint:block:block_rq_issue {@bytes = hist(args->bytes);}'
