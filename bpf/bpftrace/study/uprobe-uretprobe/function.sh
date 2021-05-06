#!/bin/bash
# rongtao 2021年5月6日

# 函数调用
function func_called() {
objdump -tT /bin/bash | grep readline
sudo bpftrace -e 'uretprobe:/bin/bash:readline { printf("read a line\n"); }'
}

function func_offset() {
objdump -tT /bin/bash | grep main
# 000000000002ec00 g    DF .text  0000000000001868  Base        main
sudo bpftrace -e 'uprobe:/bin/bash:0x2ec00 { printf("in here\n"); }'
}

function func_offset2() {
objdump -d /bin/bash
# 1046 000000000002ec00 <main@@Base>:
#	1047    2ec00:       f3 0f 1e fa             endbr64
#	1048    2ec04:       41 57                   push   %r15
#	1049    2ec06:       41 56                   push   %r14
#	1050    2ec08:       41 55                   push   %r13
sudo bpftrace -e 'uprobe:/bin/bash:main+4 { printf("in here\n"); }'

# Unsafe uprobe in the middle of the instruction: /bin/bash:main+1
sudo bpftrace -e 'uprobe:/bin/bash:main+1 { printf("in here\n"); } --unsafe'
}
