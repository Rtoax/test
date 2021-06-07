#ifndef __config_h
#define __config_h 1

#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <assert.h>

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

// Special hacked parameters to disable compaction
static const char BENCHMARK_OUTPUT_FILE[] = "/tmp/logFile";
static const bool BENCHMARK_DISABLE_COMPACTION      = false;

// See documentation in NANO_LOG.h
static const uint32_t BENCHMARK_STAGING_BUFFER_SIZE = 1<<20;
static const uint32_t BENCHMARK_OUTPUT_BUFFER_SIZE  = 1<<26;
static const uint32_t BENCHMARK_RELEASE_THRESHOLD   = 1<<19;

static const uint32_t BENCHMARK_POLL_INTERVAL_NO_WORK_US   = 1;
static const uint32_t BENCHMARK_POLL_INTERVAL_DURING_IO_US = 1;

static const uint32_t BENCHMARK_THREADS                    = 1;

// Number of iterations to run the benchmark
static const uint64_t ITERATIONS = 100000000;

// Function (or functions) to use for the NanoLog benchmark
#define BENCH_OPS NANO_LOG(NOTICE, "Simple log message with 0 parameters");;

const char BENCH_OPS_AS_A_STR[] = "NANO_LOG(NOTICE, 'Simple log message with 0 parameters');";


// Controls in what mode the compressed log file will be opened
static const int FILE_PARAMS = O_APPEND|O_RDWR|O_CREAT|O_DSYNC;

// Location of the initial log file
static const char* DEFAULT_LOG_FILE = BENCHMARK_OUTPUT_FILE;

// Determines the byte size of the per-thread StagingBuffer that decouples
// the producer logging thread from the consumer background compression
// thread. This value should be large enough to handle bursts of activity.
static const uint32_t STAGING_BUFFER_SIZE = BENCHMARK_STAGING_BUFFER_SIZE;

// Determines the size of the output buffer used to store compressed log
// messages. It should be at least 8MB large to amortize disk seeks and
// shall not be smaller than STAGING_BUFFER_SIZE.
static const uint32_t OUTPUT_BUFFER_SIZE = BENCHMARK_OUTPUT_BUFFER_SIZE;

// This invariant must be true so that we can output at least one full
// StagingBuffer per output buffer.
//    static_assert(STAGING_BUFFER_SIZE <= OUTPUT_BUFFER_SIZE,
//        "OUTPUT_BUFFER_SIZE must be greater than or "
//            "equal to the STAGING_BUFFER_SIZE");

// The threshold at which the consumer should release space back to the
// producer in the thread-local StagingBuffer. Due to the blocking nature
// of the producer when it runs out of space, a low value will incur more
// more blocking but at a shorter duration, whereas a high value will have
// the opposite effect.
static const uint32_t RELEASE_THRESHOLD = BENCHMARK_RELEASE_THRESHOLD;

// How often should the background compression thread wake up to check
// for more log messages in the StagingBuffers to compress and output.
// Due to overheads in the kernel, this number will a lower bound and
// the actual time spent sleeping may be significantly higher.
static const uint32_t POLL_INTERVAL_NO_WORK_US =
                                BENCHMARK_POLL_INTERVAL_NO_WORK_US;

// How often should the background compression thread wake up and
// check for more log messages when it's stalled waiting for an IO
// to complete. Due to overheads in the kernel, this number will
// be a lower bound and the actual time spent sleeping may be higher.
static const uint32_t POLL_INTERVAL_DURING_IO_US =
                                BENCHMARK_POLL_INTERVAL_DURING_IO_US;




#endif /* __config_h */

