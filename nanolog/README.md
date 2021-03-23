NanoLog

https://github.com/PlatformLab/NanoLog
------------------------------------------------------------------------------

Nanolog is an extremely performant nanosecond scale logging system for C++ that exposes a simple printf-like API and achieves over 80 million logs/second at a median latency of just over 7 nanoseconds.

How it achieves this insane performance is by extracting static log information at compile-time, only logging the dynamic components in runtime hotpath, and deferring formatting to an offline process. This basically shifts work out of the runtime and into the compilation and post-execution phases.

More information about the techniques used in this logging system can be found in the NanoLog Paper published in the 2018 USENIX Annual Technical Conference.
