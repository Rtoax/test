Fast Log - 低时延 LOG 系统
====================================

> 荣涛 rtoax 



# 技术背景

* 文件映射
* 线程局部变量
* metadata + binary
* aio/io_uring
* per-CPU 变量

# 安装

## 编译

```bash
cd fastlog
chmod +x compile
./compile
```

## 运行测试例

* 测试例代码

```
./test0
```

* Decoder

```
./fdecoder
```
