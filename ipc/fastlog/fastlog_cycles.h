#ifndef __fAStLOG_CYCLES_H
#define __fAStLOG_CYCLES_H 1

#ifndef __fAStLOG_H
#error "You can't include <fastlog_cycles.h> directly, #include <fastlog.h> instead."
#endif

#if !defined(__x86_64__) && defined(__i386__)
#error "Just support x86"
#endif


void 
__fastlog_cycles_init();
uint64_t 
__fastlog_get_cycles_per_sec();
double
__fastlog_cycles_to_seconds(int64_t cycles, double cyclesPerSec);
uint64_t
__fastlog_cycles_to_microseconds(uint64_t cycles, double cyclesPerSec);
uint64_t
__fastlog_cycles_to_nanoseconds(uint64_t cycles, double cyclesPerSec);
uint64_t
__fastlog_cycles_from_seconds(double seconds, double cyclesPerSec);
uint64_t
__fastlog_cycles_from_nanoseconds(uint64_t ns, double cyclesPerSec);
void
__fastlog_sleep(uint64_t us);





#endif /*<__fAStLOG_CYCLES_H>*/
