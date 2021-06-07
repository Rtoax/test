#ifndef __STAGING_BUFFER_H
#define __STAGING_BUFFER_H 1


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>


struct StagingBuffer;

struct StagingBuffer *
create_buff();

char *
reserveProducerSpace(struct StagingBuffer *buff, size_t nbytes);
void
finishReservation(struct StagingBuffer *buff, size_t nbytes);

char *
peek_buffer(struct StagingBuffer *buff, uint64_t *bytesAvailable);

void
consume_done(struct StagingBuffer *buff, uint64_t nbytes);



#endif /*<__STAGING_BUFFER_H>*/
