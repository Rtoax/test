/* Copyright (c) 2011-2018 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PERFGRAPH_CYCLES_H
#define PERFGRAPH_CYCLES_H

#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


struct Cycles {
    void (*init)();
};



/// Conversion factor between cycles and the seconds; computed by
/// Cycles::init.
static double cyclesPerSec;

/// Used for testing: if nonzero then this will be returned as the result
/// of the next call to rdtsc().
static uint64_t mockTscValue;

/// Used for testing: if nonzero, then this is used to convert from
/// cycles to seconds, instead of cyclesPerSec above.
static double mockCyclesPerSec;


/**
 * Return the current value of the fine-grain CPU cycle counter
 * (accessed via the RDTSC instruction).
 */
static __inline __attribute__((always_inline)) uint64_t
rdtsc()
{
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return (((uint64_t)hi << 32) | lo);
}

/**
 * Returns the conversion factor between cycles in seconds, using
 * a mock value for testing when appropriate.
 */
static __inline __attribute__((always_inline)) double
getCyclesPerSec()
{
    return cyclesPerSec;
}

static __inline __attribute__((always_inline)) double
perSecond(){
    return getCyclesPerSec();
}

static double toSeconds(int64_t cycles, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = getCyclesPerSec();
    return (double)(cycles)/cyclesPerSec;
}
static uint64_t fromSeconds(double seconds, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = getCyclesPerSec();
    return (uint64_t) (seconds*cyclesPerSec + 0.5);
}

static uint64_t toNanoseconds(uint64_t cycles, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = getCyclesPerSec();
    return (uint64_t) (1e09*(double)(cycles)/cyclesPerSec + 0.5);
}
static uint64_t toMicroseconds(uint64_t cycles, double cyclesPerSec)
{
    return toNanoseconds(cycles, cyclesPerSec) / 1000;
}

static uint64_t fromNanoseconds(uint64_t ns, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = getCyclesPerSec();
    return (uint64_t) ((double)(ns)*cyclesPerSec/1e09 + 0.5);
}
static void sleep(uint64_t us)
{
    uint64_t stop = rdtsc() + fromNanoseconds(1000*us, 0);
    while (rdtsc() < stop);
}



#endif  // RAMCLOUD_CYCLES_H

