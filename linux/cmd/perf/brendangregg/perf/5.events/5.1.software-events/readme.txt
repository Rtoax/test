There is a small number of fixed software events provided by perf:

# perf list

List of pre-defined events (to be used in -e):

  alignment-faults                                   [Software event]
  bpf-output                                         [Software event]
  context-switches OR cs                             [Software event]
  cpu-clock                                          [Software event]
  cpu-migrations OR migrations                       [Software event]
  dummy                                              [Software event]
  emulation-faults                                   [Software event]
  major-faults                                       [Software event]
  minor-faults                                       [Software event]
  page-faults OR faults                              [Software event]
  task-clock                                         [Software event]
[...]
These are also documented in the man page perf_event_open(2):

[...]
                   PERF_COUNT_SW_CPU_CLOCK
                          This reports the CPU clock, a  high-resolution  per-
                          CPU timer.

                   PERF_COUNT_SW_TASK_CLOCK
                          This reports a clock count specific to the task that
                          is running.

                   PERF_COUNT_SW_PAGE_FAULTS
                          This reports the number of page faults.

                   PERF_COUNT_SW_CONTEXT_SWITCHES
                          This counts context switches.  Until  Linux  2.6.34,
                          these  were all reported as user-space events, after
                          that they are reported as happening in the kernel.

                   PERF_COUNT_SW_CPU_MIGRATIONS
                          This reports the number of  times  the  process  has
                          migrated to a new CPU.

                   PERF_COUNT_SW_PAGE_FAULTS_MIN
                          This  counts the number of minor page faults.  These
                          did not require disk I/O to handle.
[...]
The kernel also supports traecpoints, which are very similar to software events, but have a different more extensible API.

Software events may have a default period. This means that when you use them for sampling, you're sampling a subset of events, not tracing every event. You can check with perf record -vv:

# perf record -vv -e context-switches /bin/true
Using CPUID GenuineIntel-6-55
------------------------------------------------------------
perf_event_attr:
  type                             1
  size                             112
  config                           0x3
  { sample_period, sample_freq }   4000
  sample_type                      IP|TID|TIME|PERIOD
  disabled                         1
  inherit                          1
  mmap                             1
  comm                             1
  freq                             1
  enable_on_exec                   1
[...]
See the perf_event_open(2) man page for a description of these fields. This default means is that the kernel adjusts the rate of sampling so that it's capturing about 4,000 context switch events per second. If you really meant to record them all, use -c 1:

# perf record -vv -e context-switches -c 1 /bin/true
Using CPUID GenuineIntel-6-55
------------------------------------------------------------
perf_event_attr:
  type                             1
  size                             112
  config                           0x3
  { sample_period, sample_freq }   1
  sample_type                      IP|TID|TIME
  disabled                         1
  inherit                          1
  mmap                             1
  comm                             1
  enable_on_exec                   1
Check the rate of events using perf stat first, so that you can estimate the volume of data you'll be capturing. Sampling a subset by default may be a good thing, especially for high frequency events like context switches.

Many other events (like tracepoints) have a default of 1 anyway. You'll encounter a non-1 default for many software and hardware events.
