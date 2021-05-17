#!/bin/bash

# 查看某个服务器全连接队列大小
ss -lnt 'sport = :8080'
# State      Recv-Q Send-Q Local Address:Port               Peer Address:Port              
# Recv-Q: 全连接队列(accept queue)的最大值，其值为min(backlog, somaxconn)
#			somaxconn: /proc/sys/net/core/somaxconn
# Send-Q: 已建立成功(状态为ESTABLISHED), 但尚未提交给应用的"TCP链接的数量"，最大值为 Send-Q + 1
