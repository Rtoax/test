/* Copyright (c) Colorado School of Mines, CST.*/
/* All rights reserved.                       */

#include <time.h>

/*
 * * `CLOCK_REALTIME` - system wide clock which measures real or wall-clock time;
 * * `CLOCK_REALTIME_COARSE` - faster version of the `CLOCK_REALTIME`;
 * * `CLOCK_MONOTONIC` - represents monotonic time since some unspecified starting point; 
 * * `CLOCK_MONOTONIC_COARSE` - faster version of the `CLOCK_MONOTONIC`;
 * * `CLOCK_MONOTONIC_RAW` - the same as the `CLOCK_MONOTONIC` but provides 
 * *					non [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) adjusted time. 
 * * `CLOCK_BOOTTIME` - the same as the `CLOCK_MONOTONIC` but plus time that the system was suspended;
 * * `CLOCK_PROCESS_CPUTIME_ID` - per-process time consumed by all threads in the process;
 * * `CLOCK_THREAD_CPUTIME_ID` - thread-specific clock.
 */

struct timespec {
   time_t   tv_sec;        /* seconds */
   long     tv_nsec;       /* nanoseconds */
};

//有多种获取当前时间的方法– gettimeofday，time，clock_gettime和其他一些特定于处理器的实现。
//    其中一些，例如gettimeofday，将从系统时钟获取时间。系统时钟可以由其他进程修改。
//    这意味着时钟可以回到过去。可以使用带有CLOCK_MONOTONIC时钟的clock_gettime来避免此问题。
//    CLOCK_MONOTONIC 参数确保我们获得一个不可设置的单调递增时钟，该时钟测量从过去某个未指定点开始的时间，
//    该时间在系统启动后不会更改[1]。确保不浪费大量CPU周期来获取当前时间也很重要。
//    获得当前时间的特定于CPU的实现将在这里有所帮助。

//The  function  clock_getres() finds the resolution (precision) of the specified clock clk_id, and, if res
//       is non-NULL, stores it in the struct timespec pointed to by res.  
int clock_gettime(clockid_t clk_id, struct timespec *tp);


//CLOCK_REALTIME
//      System-wide clock that measures real (i.e., wall-clock) time.  Setting this clock requires  appro‐
//      priate privileges.  This clock is affected by discontinuous jumps in the system time (e.g., if the
//      system administrator manually changes the clock), and by the incremental adjustments performed  by
//      adjtime(3) and NTP.
//
//CLOCK_REALTIME_COARSE (since Linux 2.6.32; Linux-specific)
//      A  faster  but less precise version of CLOCK_REALTIME.  Use when you need very fast, but not fine-
//      grained timestamps.
//
//
//CLOCK_MONOTONIC
//      Clock that cannot be set and represents monotonic  time  since  some  unspecified  starting
//      point.   This clock is not affected by discontinuous jumps in the system time (e.g., if the
//      system administrator manually changes the  clock),  but  is  affected  by  the  incremental
//      adjustments performed by adjtime(3) and NTP.
//
//CLOCK_MONOTONIC_COARSE (since Linux 2.6.32; Linux-specific)
//      A faster but less precise version of CLOCK_MONOTONIC.  Use when you need very fast, but not
//      fine-grained timestamps.
//
//CLOCK_MONOTONIC_RAW (since Linux 2.6.28; Linux-specific)
//      Similar to CLOCK_MONOTONIC, but provides access to a raw hardware-based time  that  is  not
//      subject to NTP adjustments or the incremental adjustments performed by adjtime(3).
//
//CLOCK_BOOTTIME (since Linux 2.6.39; Linux-specific)
//      Identical  to  CLOCK_MONOTONIC,  except  it  also includes any time that the system is sus‐
//      pended.  This allows applications to get a suspend-aware monotonic clock without having  to
//      deal  with  the complications of CLOCK_REALTIME, which may have discontinuities if the time
//      is changed using settimeofday(2).
//
//CLOCK_PROCESS_CPUTIME_ID
//      High-resolution per-process timer from the CPU.
//
//CLOCK_THREAD_CPUTIME_ID
//      Thread-specific CPU-time clock.





//Link with -lrt (only for glibc versions before 2.17).
SYSCALL_DEFINE2(clock_gettime, const clockid_t, which_clock,
		struct __kernel_timespec __user *, tp)
{
	const struct k_clock *kc = clockid_to_kclock(which_clock);
	struct timespec64 kernel_tp;
	int error;

	if (!kc)
		return -EINVAL;

	error = kc->clock_get_timespec(which_clock, &kernel_tp);

	if (!error && put_timespec64(&kernel_tp, tp))
		error = -EFAULT;

	return error;
}

