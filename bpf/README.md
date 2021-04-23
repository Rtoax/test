# bpf

Berkeley Packet Filter

```
        _____________
       /\____    ____\
       \/___ \   \___/    _____     _____     ___  ___
            \ \   \      / ___ \   / ___ \   /\  \/  /
             \ \   \    /\ \_/\ \ /\ \_/\ \  \ \    /
              \ \   \   \ \ \\_\ \\ \ \\_\ \  \/    \
  R            \ \___\   \ \____ / \ \___  _\ /  /\  \
                \/___/    \/____/   \/___/\_//__/\_\__\
```

## Front end 介绍


* `BPF-bytecode`: Brutal Precise control	Insanely difficult	Kernel source: struct bpf_insn prog in samples/bpf/sock_example.c
* `C`: Hard	Build stand-alone binaries	Difficult	Kernel source: samples/bpf/tracex1_kern.c and samples/bpf/tracex1_user.c
* `perf`: Hard	Use perf's capabilities: custom events, stack walking	Difficult, not yet well documented	Section below: 7. perf.
* `bcc`: Moderate	Custom output, CO-RE binaries, large community, production use (e.g., Facebook, Netflix)	Verbose	Section below: 5. bcc.
* `bpftrace`: Easy	Powerful one-liners, many capabilities, growing community, production use (e.g., Netflix, Facebook)	Limited control of code and output	Section below: 6. bpftrace.
* `ply`: Easy	Powerful one-liners, small binary, for embedded	Limited control of code and output	github: github.com/iovisor/ply.



## Back end 介绍



# /proc

* /sys/kernel/debug/tracing/events/

