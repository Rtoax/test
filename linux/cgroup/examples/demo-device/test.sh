#!/bin/bash

if [ $UID != 0 ]; then
	echo "使用 root 执行。"
	exit 1
fi

sudo mkdir /sys/fs/cgroup/devices/cgroup_test_group

# ls -l /dev/tty
# crw-rw-rw- 1 root tty 5, 0 Dec  3 22:48 /dev/tty
# 字符设备 主设备号:此设备号 写权限
echo "c 5:0 w" > /sys/fs/cgroup/devices/cgroup_test_group/devices.deny

# 禁止使用 这个设备
echo $(pidof -x cgroup_test_script.sh) > /sys/fs/cgroup/devices/cgroup_test_group/tasks

exit
