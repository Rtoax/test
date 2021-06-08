#!/bin/bash

# -f: 生成折叠格式的输出
profile -p $(pgrep mysqld) -f 30 > profile01.txt.out
flamegraph.pl --width=80 --title="CPU Falme Graph" < profile01.txt.out \
					  > /work/workspace/profile01.txt.out.svg
