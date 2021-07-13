# dump-x86_64-pagetable

Linux Kernel module to dump the page tables currently used by the CPU on intel 64bit systems with 4 level paging.

Tested on Kernel 4.9 and 4.4.


### Example
```
# cat /sys/kernel/debug/corny/intel_pagetables /proc/self/maps
```

The `/proc/self/maps` are
```
55778bc76000-55778bc7e000 r-xp 00000000 08:01 261639                     /bin/cat
55778be7d000-55778be7e000 r--p 00007000 08:01 261639                     /bin/cat
55778be7e000-55778be7f000 rw-p 00008000 08:01 261639                     /bin/cat
55778d594000-55778d5d8000 rw-p 00000000 00:00 0                          [heap]
7fce38693000-7fce38828000 r-xp 00000000 08:01 919180                     /lib/x86_64-linux-gnu/libc-2.24.so
7fce38828000-7fce38a28000 ---p 00195000 08:01 919180                     /lib/x86_64-linux-gnu/libc-2.24.so
7fce38a28000-7fce38a2c000 r--p 00195000 08:01 919180                     /lib/x86_64-linux-gnu/libc-2.24.so
7fce38a2c000-7fce38a2e000 rw-p 00199000 08:01 919180                     /lib/x86_64-linux-gnu/libc-2.24.so
7fce38a2e000-7fce38a32000 rw-p 00000000 00:00 0 
7fce38a32000-7fce38a55000 r-xp 00000000 08:01 919176                     /lib/x86_64-linux-gnu/ld-2.24.so
7fce38aaf000-7fce38c4a000 r--p 00000000 08:01 132354                     /usr/lib/locale/locale-archive
7fce38c4a000-7fce38c4c000 rw-p 00000000 00:00 0 
7fce38c52000-7fce38c55000 rw-p 00000000 00:00 0 
7fce38c55000-7fce38c56000 r--p 00023000 08:01 919176                     /lib/x86_64-linux-gnu/ld-2.24.so
7fce38c56000-7fce38c57000 rw-p 00024000 08:01 919176                     /lib/x86_64-linux-gnu/ld-2.24.so
7fce38c57000-7fce38c58000 rw-p 00000000 00:00 0 
7ffcc944d000-7ffcc946e000 rw-p 00000000 00:00 0                          [stack]
7ffcc9523000-7ffcc9525000 r--p 00000000 00:00 0                          [vvar]
7ffcc9525000-7ffcc9527000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
```

The module prints the outer two levels of page tables. As we can see, the userspace addresses agree.
```
mymod: Hello, world
(GenuineIntel) x86_phys_bits 36
cr3: 0xb5bbc000
paging according to Tab 4-12 p. 2783 intel dev manual (March 2017)
No protection keys enabled (this is normal)
page table in virtual memory at ffff987335bbc000
entry 00000000b7a2f067
v 0000550000000000 0000557fffffffff W U   A
  v 0000557780000000 00005577bfffffff W U   A
entry 00000000b8b14067
v 00007f8000000000 00007fffffffffff W U   A
  v 00007fce00000000 00007fce3fffffff W U   A
  v 00007ffcc0000000 00007ffcffffffff W U   A
entry 0000000060f2e067
v ffff980000000000 ffff987fffffffff W U   A
  v ffff987280000000 ffff9872bfffffff W U   A
  v ffff9872c0000000 ffff9872ffffffff W U   A
  v ffff987300000000 ffff98733fffffff W U   A
entry 00000000bc084067
v ffffbe8000000000 ffffbeffffffffff W U   A
  v ffffbe8e80000000 ffffbe8ebfffffff W U   A
entry 00000000bc093067
v ffffde8000000000 ffffdeffffffffff W U   A
  v ffffde8e40000000 ffffde8e7fffffff W U   A
entry 00000000bcbe8067
v fffff68000000000 fffff6ffffffffff W U   A
  v fffff6ecc0000000 fffff6ecffffffff W U   A
entry 0000000060e86067
v ffffff0000000000 ffffff7fffffffff W U   A
  v ffffff7c00000000 ffffff7c3fffffff R K   A
  v ffffff7c40000000 ffffff7c7fffffff R K   A
  v ffffff7c80000000 ffffff7cbfffffff R K   A
  v ffffff7cc0000000 ffffff7cffffffff R K   A
entry 0000000060c0a067
v ffffff8000000000 ffffffffffffffff W U   A
  v ffffffff80000000 ffffffffbfffffff W K   A
  v ffffffffc0000000 ffffffffffffffff W U   A
```

### Bugs

*Yes!* Don't run this on your production machine! This is just a test module!

What could possibly go wrong? I'm just dumping the page table starting from the CPUs `cr3`. My module does not attempt any locking. While dumping the page tables, the kernel may already has changed the mappings from under my feet. This would mean that my module dereferences invalid memory.

### How would I do this the correct way?
Use `/proc/self/maps` to inspect the userpsace memory layout.
Use `/sys/kernel/debug/kernel_page_tables` to inspect the kernel page tables. That module uses the Kernel API correctly and is [extremely simple](http://elixir.free-electrons.com/linux/v4.11.4/source/arch/x86/mm/debug_pagetables.c).




