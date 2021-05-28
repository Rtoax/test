#!/bin/bash

# 跟踪点细节信息
bpftrace -lv t:scsi:scsi_dispatch_cmd_done
