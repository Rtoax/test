docker 快速入门

========================

> 荣涛 2021年6月10日

# docker 命令的生命周期

```bash
# 加载镜像
docker load --input centos7-minimal.tar
# 运行容器
docker run -d -p 10022:22 centos7-ssh:latest /usr/sbin/sshd -D
# 查看容器
docker ps
# 执行命令
docker exec -ti 01c9218dd2a2 /bin/bash
# 停止容器
docker stop 01c9218dd2a2
```

# /sys/fs/cgroups

## CPU

/sys/fs/cgroups/cpu,cpuacct/docker/[容器ID]/*

