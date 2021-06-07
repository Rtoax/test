#include <assert.h>
#include <stdlib.h>
#include "staging-buffer.h"

#define STAGING_BUFFER_SIZE 4096

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


struct StagingBuffer *
create_buff()
{
    size_t i;
    struct StagingBuffer *staging_buf = NULL;
    
    staging_buf = malloc(sizeof(struct StagingBuffer));
    assert(staging_buf && "malloc error");

    staging_buf->endOfRecordedSpace = staging_buf->storage + STAGING_BUFFER_SIZE;
    staging_buf->minFreeSpace = 0;

    staging_buf->cyclesProducerBlocked = 0;
    staging_buf->numTimesProducerBlocked = 0;
    staging_buf->numAllocations = 0;

    staging_buf->consumerPos = staging_buf->storage;
    staging_buf->shouldDeallocate = false;
    staging_buf->id = 1;//bufferId;
    staging_buf->producerPos = staging_buf->storage;


    for (i = 0; i < 20; ++i)
    {
        staging_buf->cyclesProducerBlockedDist[i] = 0;
    }
    return staging_buf;
}


/**
* Attempt to reserve contiguous space for the producer without making it
* visible to the consumer (See reserveProducerSpace).
*
* This is the slow path of reserveProducerSpace that checks for free space
* within storage[] that involves touching variable shared with the compression
* thread and thus causing potential cache-coherency delays.
*
* \param nbytes
*      Number of contiguous bytes to reserve.
*
* \param blocking
*      Test parameter that indicates that the function should
*      return with a nullptr rather than block when there's
*      not enough space.
*
* \return
*      A pointer into storage[] that can be written to by the producer for
*      at least nbytes.
*/
char *
reserveSpaceInternal(struct StagingBuffer *buff, size_t nbytes, bool blocking) {
    const char *endOfBuffer = buff->storage + STAGING_BUFFER_SIZE;

    // There's a subtle point here, all the checks for remaining
    // space are strictly < or >, not <= or => because if we allow
    // the record and print positions to overlap, we can't tell
    // if the buffer either completely full or completely empty.
    // Doing this check here ensures that == means completely empty.
    while (buff->minFreeSpace <= nbytes) {
        // Since consumerPos can be updated in a different thread, we
        // save a consistent copy of it here to do calculations on
        char *cachedConsumerPos = buff->consumerPos;

        if (cachedConsumerPos <= buff->producerPos) {
            buff->minFreeSpace = endOfBuffer - buff->producerPos;

            if (buff->minFreeSpace > nbytes)
                break;

            // Not enough space at the end of the buffer; wrap around
            buff->endOfRecordedSpace = buff->producerPos;

            // Prevent the roll over if it overlaps the two positions because
            // that would imply the buffer is completely empty when it's not.
            if (cachedConsumerPos != buff->storage) {
                // prevents producerPos from updating before endOfRecordedSpace
                asm volatile("sfence":::"memory");
                buff->producerPos = buff->storage;
                buff->minFreeSpace = cachedConsumerPos - buff->producerPos;
            }
        } else {
            buff->minFreeSpace = cachedConsumerPos - buff->producerPos;
        }
        // Needed to prevent infinite loops in tests
        if (!blocking && buff->minFreeSpace <= nbytes)
            return NULL;
    }

    ++buff->numTimesProducerBlocked;
    return buff->producerPos;
}



/**
 * Attempt to reserve contiguous space for the producer without
 * making it visible to the consumer. The caller should invoke
 * finishReservation() before invoking reserveProducerSpace()
 * again to make the bytes reserved visible to the consumer.
 *
 * This mechanism is in place to allow the producer to initialize
 * the contents of the reservation before exposing it to the
 * consumer. This function will block behind the consumer if
 * there's not enough space.
 *
 * \param nbytes
 *      Number of bytes to allocate
 *
 * \return
 *      Pointer to at least nbytes of contiguous space
 */
char *
reserveProducerSpace(struct StagingBuffer *buff, size_t nbytes) 
{
    ++buff->numAllocations;

    // Fast in-line path
    if (nbytes < buff->minFreeSpace)
        return buff->producerPos;

    // Slow allocation
    return reserveSpaceInternal(buff, nbytes, true);
}

void
finishReservation(struct StagingBuffer *buff, size_t nbytes) 
{
    assert(nbytes < buff->minFreeSpace);
    assert(buff->producerPos + nbytes <
           buff->storage + STAGING_BUFFER_SIZE);

    asm volatile("sfence":::"memory"); // Ensures producer finishes writes before bump
    buff->minFreeSpace -= nbytes;
    buff->producerPos += nbytes;
}


/**
* Peek at the data available for consumption within the stagingBuffer.
* The consumer should also invoke consume() to release space back
* to the producer. This can and should be done piece-wise where a
* large peek can be consume()-ed in smaller pieces to prevent blocking
* the producer.
*
* \param[out] bytesAvailable
*      Number of bytes consumable
* \return
*      Pointer to the consumable space
*/
char *
peek_buffer(struct StagingBuffer *buff, uint64_t *bytesAvailable)
{
    // Save a consistent copy of producerPos
    char *cachedProducerPos = buff->producerPos;

    if (cachedProducerPos < buff->consumerPos) {
        asm volatile("lfence":::"memory"); // Prevent reading new producerPos but old endOf...
        *bytesAvailable = buff->endOfRecordedSpace - buff->consumerPos;

        if (*bytesAvailable > 0)
            return buff->consumerPos;

        // Roll over
        buff->consumerPos = buff->storage;
    }

    *bytesAvailable = cachedProducerPos - buff->consumerPos;
    return buff->consumerPos;
}

/**
 * Consumes the next nbytes in the StagingBuffer and frees it back
 * for the producer to reuse. nbytes must be less than what is
 * returned by peek().
 *
 * \param nbytes
 *      Number of bytes to return back to the producer
 */
void
consume_done(struct StagingBuffer *buff, uint64_t nbytes)
{
    asm volatile("lfence":::"memory"); // Make sure consumer reads finish before bump
    buff->consumerPos += nbytes;
}

