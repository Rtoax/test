Visualizations
perf_events has a builtin visualization: timecharts, as well as text-style visualization via its text user interface (TUI) and tree reports. The following two sections show visualizations of my own: flame graphs and heat maps. The software I'm using is open source and on github, and produces these from perf_events collected data.

7.1. Flame Graphs
Flame Graphs can be produced from perf_events profiling data using the FlameGraph tools software. This visualizes the same data you see in perf report, and works with any perf.data file that was captured with stack traces (-g).

Example
This example CPU flame graph shows a network workload for the 3.2.9-1 Linux kernel, running as a KVM instance (SVG, PNG):

Flame Graphs show the sample population across the x-axis, and stack depth on the y-axis. Each function (stack frame) is drawn as a rectangle, with the width relative to the number of samples. See the CPU Flame Graphs page for the full description of how these work.

You can use the mouse to explore where kernel CPU time is spent, quickly quantifying code-paths and determining where performance tuning efforts are best spent. This example shows that most time was spent in the vp_notify() code-path, spending 70.52% of all on-CPU samples performing iowrite16(), which is handled by the KVM hypervisor. This information has been extremely useful for directing KVM performance efforts.

A similar network workload on a bare metal Linux system looks quite different, as networking isn't processed via the virtio-net driver, for a start.

Generation
The example flame graph was generated using perf_events and the FlameGraph tools:

# git clone https://github.com/brendangregg/FlameGraph  # or download it from github
# cd FlameGraph
# perf record -F 99 -ag -- sleep 60
# perf script | ./stackcollapse-perf.pl > out.perf-folded
# cat out.perf-folded | ./flamegraph.pl > perf-kernel.svg
The first perf command profiles CPU stacks, as explained earlier. I adjusted the rate to 99 Hertz here; I actually generated the flame graph from a 1000 Hertz profile, but I'd only use that if you had a reason to go faster, which costs more in overhead. The samples are saved in a perf.data file, which can be viewed using perf report:

# perf report --stdio
[...]
# Overhead          Command          Shared Object                               Symbol
# ........  ...............  .....................  ...................................
#
    72.18%            iperf  [kernel.kallsyms]      [k] iowrite16
                      |
                      --- iowrite16
                         |          
                         |--99.53%-- vp_notify
                         |          virtqueue_kick
                         |          start_xmit
                         |          dev_hard_start_xmit
                         |          sch_direct_xmit
                         |          dev_queue_xmit
                         |          ip_finish_output
                         |          ip_output
                         |          ip_local_out
                         |          ip_queue_xmit
                         |          tcp_transmit_skb
                         |          tcp_write_xmit
                         |          |          
                         |          |--98.16%-- tcp_push_one
                         |          |          tcp_sendmsg
                         |          |          inet_sendmsg
                         |          |          sock_aio_write
                         |          |          do_sync_write
                         |          |          vfs_write
                         |          |          sys_write
                         |          |          system_call
                         |          |          0x369e40e5cd
                         |          |          
                         |           --1.84%-- __tcp_push_pending_frames
[...]
This tree follows the flame graph when reading it top-down. When using -g/--call-graph (for "caller", instead of the "callee" default), it generates a tree that follows the flame graph when read bottom-up. The hottest stack trace in the flame graph (@70.52%) can be seen in this perf call graph as the product of the top three nodes (72.18% x 99.53% x 98.16%).

The perf report tree (and the ncurses navigator) do an excellent job at presenting this information as text. However, with text there are limitations. The output often does not fit in one screen (you could say it doesn't need to, if the bulk of the samples are identified on the first page). Also, identifying the hottest code paths requires reading the percentages. With the flame graph, all the data is on screen at once, and the hottest code-paths are immediately obvious as the widest functions.

For generating the flame graph, the perf script command dumps the stack samples, which are then aggregated by stackcollapse-perf.pl and folded into single lines per-stack. That output is then converted by flamegraph.pl into the SVG. I included a gratuitous "cat" command to make it clear that flamegraph.pl can process the output of a pipe, which could include Unix commands to filter or preprocess (grep, sed, awk).

Piping
A flame graph can be generated directly by piping all the steps:

# perf script | ./stackcollapse-perf.pl | ./flamegraph.pl > perf-kernel.svg
In practice I don't do this, as I often re-run flamegraph.pl multiple times, and this one-liner would execute everything multiple times. The output of perf script can be dozens of Mbytes, taking many seconds to process. By writing stackcollapse-perf.pl to a file, you've cached the slowest step, and can also edit the file (vi) to delete unimportant stacks, such as CPU idle threads.

Filtering
The one-line-per-stack output of stackcollapse-perf.pl is also convenient for grep(1). Eg:

# perf script | ./stackcollapse-perf.pl > out.perf-folded

# grep -v cpu_idle out.perf-folded | ./flamegraph.pl > nonidle.svg

# grep ext4 out.perf-folded | ./flamegraph.pl > ext4internals.svg

# egrep 'system_call.*sys_(read|write)' out.perf-folded | ./flamegraph.pl > rw.svg
I frequently elide the cpu_idle threads in this way, to focus on the real threads that are consuming CPU resources. If I miss this step, the cpu_idle threads can often dominate the flame graph, squeezing the interesting code paths.

Note that it would be a little more efficient to process the output of perf report instead of perf script; better still, perf report could have a report style (eg, "-g folded") that output folded stacks directly, obviating the need for stackcollapse-perf.pl. There could even be a perf mode that output the SVG directly (which wouldn't be the first one; see perf-timechart), although, that would miss the value of being able to grep the folded stacks (which I use frequently).

There are more examples of perf_events CPU flame graphs on the CPU flame graph page, including a summary of these instructions. I have also shared an example of using perf for a Block Device I/O Flame Graph.
