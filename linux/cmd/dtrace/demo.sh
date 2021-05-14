#!/bin/bash

# malloc 调用的请求大小
sudo  /usr/sbin/dtrace -n 'pid$target::malloc:entry {@["requested butes"] = quantize(arg0); }' -p 1
