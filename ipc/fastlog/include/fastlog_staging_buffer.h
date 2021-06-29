#ifndef __STAGING_BUFFER_H
#define __STAGING_BUFFER_H 1

#ifndef __fAStLOG_H
#error "You can't include <fastlog_staging_buffer.h> directly, #include <fastlog.h> instead."
#endif


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>

#include <fastlog_utils.h>

struct StagingBuffer;


#define STAGING_BUFFER_SIZE 8192

struct StagingBuffer {

    // Position within storage[] where the producer may place new data
    char *producerPos;

    // Marks the end of valid data for the consumer. Set by the producer
    // on a roll-over
    char *endOfRecordedSpace;

    // Lower bound on the number of bytes the producer can allocate w/o
    // rolling over the producerPos or stalling behind the consumer
    uint64_t minFreeSpace;

    // Number of cycles producer was blocked while waiting for space to
    // free up in the StagingBuffer for an allocation.
    uint64_t cyclesProducerBlocked;

    // Number of times the producer was blocked while waiting for space
    // to free up in the StagingBuffer for an allocation
    uint32_t numTimesProducerBlocked;

    // Number of alloc()'s performed
    uint64_t numAllocations;

    // Distribution of the number of times Producer was blocked
    // allocating space in 10ns increments. The last slot includes
    // all times greater than the last increment.
    uint32_t cyclesProducerBlockedDist[20];

    // An extra cache-line to separate the variables that are primarily
    // updated/read by the producer (above) from the ones by the
    // consumer(below)
    char cacheLineSpacer[2*64];

    // Position within the storage buffer where the consumer will consume
    // the next bytes from. This value is only updated by the consumer.
    char* volatile consumerPos;

    // Indicates that the thread owning this StagingBuffer has been
    // destructed (i.e. no more messages will be logged to it) and thus
    // should be cleaned up once the buffer has been emptied by the
    // compression thread.
    bool shouldDeallocate;

    // Uniquely identifies this StagingBuffer for this execution. It's
    // similar to ThreadId, but is only assigned to threads that NANO_LOG).
    uint32_t id;

    // Backing store used to implement the circular queue
    char storage[STAGING_BUFFER_SIZE];
};


fl_inline void
ensureStagingBufferAllocated();


fl_inline struct StagingBuffer *
create_buff();

fl_inline char *
reserveProducerSpace(struct StagingBuffer *buff, size_t nbytes);
fl_inline void
finishReservation(struct StagingBuffer *buff, size_t nbytes);

fl_inline char *
peek_buffer(struct StagingBuffer *buff, uint64_t *bytesAvailable);

fl_inline void
consume_done(struct StagingBuffer *buff, uint64_t nbytes);



#endif /*<__STAGING_BUFFER_H>*/

