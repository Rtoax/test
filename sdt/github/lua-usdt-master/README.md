lua-usdt
========

Lua bindings for libusdt[1].

Build with first:

```
  cd libusdt
  make
  cd ..
```

and then:

```
  luarocks --local build usdt-0.1-1.rockspec USDT_INCDIR=libusdt
```

Test:

```
  sudo dtrace -Z \
     -n 'lua*:lmod:foo:cprobe{ printf("%s %s %s %s %s %s %s %s %s %s", copyinstr(arg0), copyinstr(arg1), \
         copyinstr(arg2), copyinstr(arg3), copyinstr(arg4), copyinstr(arg5), copyinstr(arg6), \
         copyinstr(arg7), copyinstr(arg8), copyinstr(arg9)) }' \
     -n 'lua*:lmod:foo:iprobe{ printf("%d %d %d %d %d %d %d %d %d %d", arg0, arg1, arg2, arg3, arg4, \
         arg5, arg6, arg7, arg8, arg9) }' \
     -c 'lua test.lua'
```

The output of this should be along the lines of:

```
dtrace: description 'lua*:lmod:foo:cprobe' matched 0 probes
dtrace: description 'lua*:lmod:foo:iprobe' matched 0 probes
dtrace: pid 33227 has exited
CPU     ID                    FUNCTION:NAME
  0   3950                       foo:iprobe 1 2 3 4 5 6 7 8 9 10
  0   3951                       foo:cprobe a b c d e f g h i j
```

[1] - https://github.com/chrisa/libusdt
