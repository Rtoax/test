#include <fastlog.h>
#include <fastlog_utils.h>
#include <fastlog_staging_buffer.h>

extern __thread struct StagingBuffer *stagingBuffer;

extern struct StagingBuffer *threadBuffers[1024];
extern fastlog_atomic64_t  stagingBufferId;

int __bg_add_buffer_event_to_epoll();


inline void
ensureStagingBufferAllocated()
{
    /**
     *  创建一个 buffer
     */
    if (unlikely(stagingBuffer == NULL)) {
        stagingBuffer = create_buff(fastlog_atomic64_read(&stagingBufferId));
        threadBuffers[fastlog_atomic64_read(&stagingBufferId)] = stagingBuffer;
        fastlog_atomic64_inc(&stagingBufferId);
    }
    /**
     *  创建 buffer 对应的 eventfd，并将其 添加 至 后台线程轮询的 epoll 中
     */
    __bg_add_buffer_event_to_epoll();
    
}


inline struct StagingBuffer *
create_buff(uint32_t bufferId)
{
    size_t i;
    struct StagingBuffer *staging_buf = NULL;

//    printf("create staging buffer, buffer id = %d\n", bufferId);
    
    staging_buf = malloc(sizeof(struct StagingBuffer));
    assert(staging_buf && "malloc error");

    staging_buf->endOfRecordedSpace = staging_buf->storage + STAGING_BUFFER_SIZE;
    staging_buf->minFreeSpace = 0;

    staging_buf->cyclesProducerBlocked = 0;
    staging_buf->numTimesProducerBlocked = 0;
    staging_buf->numAllocations = 0;

    staging_buf->consumerPos = staging_buf->storage;
    staging_buf->shouldDeallocate = false;
    staging_buf->id = bufferId;
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
inline char *
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
inline char *
reserveProducerSpace(struct StagingBuffer *buff, size_t nbytes) 
{
    ++buff->numAllocations;

    // Fast in-line path
    if (nbytes < buff->minFreeSpace)
        return buff->producerPos;

    // Slow allocation
    return reserveSpaceInternal(buff, nbytes, true);
}

inline void
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
inline char *
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
inline void
consume_done(struct StagingBuffer *buff, uint64_t nbytes)
{
    asm volatile("lfence":::"memory"); // Make sure consumer reads finish before bump
    buff->consumerPos += nbytes;
}

