#!/bin/bash
# rongtao 2021年5月6日

# 跟踪 数据结构 
sudo bpftrace -e 'struct Foo {int m;} uprobe:./a.out:func { $s = (struct Foo *) arg0; @x = $s->m; exit(); }'
sudo bpftrace -e 'struct Foo {char *a; char b[4]; uint8_t c[4]; int d[4];} uprobe:./a.out:func { $s = (struct Foo *) arg0; @x = $s->d[2]; exit(); }'
