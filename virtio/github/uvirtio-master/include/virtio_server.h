#ifndef VIRTIO_SERVER_H_
#define VIRTIO_SERVER_H_

#include "virtio.h"

#include <linux/virtio_ring.h>

#include <linux/types.h>
#include <sys/uio.h>
#include <stdbool.h>
#include <linux/kernel.h>

#define to_vvq(_vq) container_of(_vq, struct vring_virtqueue, vq)

/**
 * operations for virtqueue
 * virtqueue_add_buf: expose buffer to other end
 *	vq: the struct virtqueue we're talking about.
 *	sg: the description of the buffer(s).
 *	out_num: the number of sg readable by other side
 *	in_num: the number of sg which are writable (after readable ones)
 *	data: the token identifying the buffer.
 *	gfp: how to do memory allocations (if necessary).
 *      Returns remaining capacity of queue (sg segments) or a negative error.
 * virtqueue_kick: update after add_buf
 *	vq: the struct virtqueue
 *	After one or more add_buf calls, invoke this to kick the other side.
 * virtqueue_get_buf: get the next used buffer
 *	vq: the struct virtqueue we're talking about.
 *	len: the length written into the buffer
 *	Returns NULL or the "data" token handed to add_buf.
 * virtqueue_disable_cb: disable callbacks
 *	vq: the struct virtqueue we're talking about.
 *	Note that this is not necessarily synchronous, hence unreliable and only
 *	useful as an optimization.
 * virtqueue_enable_cb: restart callbacks after disable_cb.
 *	vq: the struct virtqueue we're talking about.
 *	This re-enables callbacks; it returns "false" if there are pending
 *	buffers in the queue, to detect a possible race between the driver
 *	checking for more work, and enabling callbacks.
 * virtqueue_enable_cb_delayed: restart callbacks after disable_cb.
 *	vq: the struct virtqueue we're talking about.
 *	This re-enables callbacks but hints to the other side to delay
 *	interrupts until most of the available buffers have been processed;
 *	it returns "false" if there are many pending buffers in the queue,
 *	to detect a possible race between the driver checking for more work,
 *	and enabling callbacks.
 * virtqueue_detach_unused_buf: detach first unused buffer
 * 	vq: the struct virtqueue we're talking about.
 * 	Returns NULL or the "data" token handed to add_buf
 * virtqueue_get_vring_size: return the size of the virtqueue's vring
 *	vq: the struct virtqueue containing the vring of interest.
 *	Returns the size of the vring.
 *
 * Locking rules are straightforward: the driver is responsible for
 * locking.  No two operations may be invoked simultaneously, with the exception
 * of virtqueue_disable_cb.
 *
 * All operations can be called in any context.
 */

struct virtqueue *vring_new_virtqueue(unsigned int num,
				      unsigned int vring_align,
				      struct virtio_device *vdev,
				      void *vq_addr,
				      void *pages,
				      void (*notify)(struct virtqueue *),
				      void (*callback)(struct virtqueue *),
				      const char *name);

int virtqueue_add_buf(struct virtqueue *_vq,
			  struct iovec sg[],
			  unsigned int out,
			  unsigned int in,
			  void *data);

void virtqueue_kick(struct virtqueue *_vq);

void virtqueue_kick(struct virtqueue *vq);

void *virtqueue_get_buf(struct virtqueue *vq, unsigned int *len);

void virtqueue_disable_cb(struct virtqueue *vq);

bool virtqueue_enable_cb(struct virtqueue *vq);

bool virtqueue_enable_cb_delayed(struct virtqueue *vq);

void *virtqueue_detach_unused_buf(struct virtqueue *vq);

unsigned int virtqueue_get_vring_size(struct virtqueue *vq);

#endif