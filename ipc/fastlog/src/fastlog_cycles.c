#include <sys/time.h>

#include <fastlog.h>
#include <fastlog_cycles.h>



static uint64_t __cycles_per_sec = 0;


void __fastlog_cycles_init() 
{
    if (__cycles_per_sec != 0)
        return;

    struct timeval startTime, stopTime;
    uint64_t startCycles, stopCycles, micros;
    double oldCycles;

    oldCycles = 0;
    while (1) {
        if (gettimeofday(&startTime, NULL) != 0) {
            fprintf(stderr, "Cycles::init couldn't read clock: %s", strerror(errno));
            assert(0);
        }
        startCycles = __fastlog_rdtsc();
        while (1) {
            if (gettimeofday(&stopTime, NULL) != 0) {
                fprintf(stderr, "Cycles::init couldn't read clock: %s", strerror(errno));
                assert(0);
            }
            stopCycles = __fastlog_rdtsc();
            micros = (stopTime.tv_usec - startTime.tv_usec) +
                    (stopTime.tv_sec - startTime.tv_sec)*1000000;
            if (micros > 10000) {
                __cycles_per_sec = (double)(stopCycles - startCycles);
                __cycles_per_sec = 1000000.0*__cycles_per_sec/(double)(micros);
//                printf("cal __cycles_per_sec %ld\n", __cycles_per_sec);
                break;
            }
        }
        double delta = __cycles_per_sec/100000.0;
        if ((oldCycles > (__cycles_per_sec - delta)) &&
                (oldCycles < (__cycles_per_sec + delta))) {
//            printf("cal done __cycles_per_sec %ld\n", __cycles_per_sec);
            return;
        }
        oldCycles = __cycles_per_sec;
    }

//    printf("ret __cycles_per_sec = %ld\n", __cycles_per_sec);
}

uint64_t 
__fastlog_get_cycles_per_sec()
{
    return __cycles_per_sec;
}





double
__fastlog_cycles_to_seconds(int64_t cycles, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = __fastlog_get_cycles_per_sec();
    return (double)(cycles)/cyclesPerSec;
}

uint64_t
__fastlog_cycles_to_microseconds(uint64_t cycles, double cyclesPerSec)
{
    return __fastlog_cycles_to_nanoseconds(cycles, cyclesPerSec) / 1000;
}


uint64_t
__fastlog_cycles_to_nanoseconds(uint64_t cycles, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = __fastlog_get_cycles_per_sec();
    return (uint64_t) (1e09*(double)(cycles)/cyclesPerSec + 0.5);
}

uint64_t
__fastlog_cycles_from_seconds(double seconds, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = __fastlog_get_cycles_per_sec();
    return (uint64_t) (seconds*cyclesPerSec + 0.5);
}


uint64_t
__fastlog_cycles_from_nanoseconds(uint64_t ns, double cyclesPerSec)
{
    if (cyclesPerSec == 0)
        cyclesPerSec = __fastlog_get_cycles_per_sec();
    return (uint64_t) ((double)(ns)*cyclesPerSec/1e09 + 0.5);
}

void
__fastlog_sleep(uint64_t us)
{
    uint64_t stop = __fastlog_rdtsc() + __fastlog_cycles_from_nanoseconds(1000*us, __fastlog_get_cycles_per_sec());
    while (__fastlog_rdtsc() < stop);
}

