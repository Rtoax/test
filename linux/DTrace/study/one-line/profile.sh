#!/bin/bash

# 按 99Hz采样内核级栈
dtrace -n 'profile:::profile-99 { @[stack()] = count();  }'
