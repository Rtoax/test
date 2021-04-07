#include <utils.h>
#include "virtio_server.h"

#include <stdio.h>
#include <linux/list.h>
#include <errno.h>
#include <assert.h>

#ifdef DEBUG
/* For development, we want to crash whenever the ring is screwed. */
#define BAD_RING(_vq, fmt, args...)				\
	do {							\
		dev_err(&(_vq)->vq.vdev->dev,			\
			"%s:"fmt, (_vq)->vq.name, ##args);	\
		BUG();						\
	} while (0)
/* Caller is supposed to guarantee no reentry. */
#define START_USE(_vq)						\
	do {							\
		if ((_vq)->in_use)				\
			panic("%s:in_use = %i\n",		\
			      (_vq)->vq.name, (_vq)->in_use);	\
		(_vq)->in_use = __LINE__;			\
	} while (0)
#define END_USE(_vq) \
	do { BUG_ON(!(_vq)->in_use); (_vq)->in_use = 0; } while(0)
#else
#define BAD_RING(_vq, fmt, args...)				\
	do {							\
		dev_err(&_vq->vq.vdev->dev,			\
			"%s:"fmt, (_vq)->vq.name, ##args);	\
		(_vq)->broken = true;				\
	} while (0)
#define START_USE(vq)
#define END_USE(vq)
#endif

struct virtqueue *vring_new_virtqueue(unsigned int num,
				      unsigned int vring_align,
				      struct virtio_device *vdev,
				      void *vq_addr,
				      void *pages,
				      void (*notify)(struct virtqueue *),
				      void (*callback)(struct virtqueue *),
				      const char *name)
{
    linfo("NUM %d, align %d, virtqueue addr 0x%p, pages 0x%p, name %s\n",
            num, vring_align, vq_addr, pages, name);

	struct vring_virtqueue *vq = vq_addr;
	unsigned int i;

	/* We assume num is a power of 2. */
	if (num & (num - 1)) {
		printf("Bad virtqueue length %u\n", num);
		return NULL;
	}

	vring_init(&vq->vring, num, pages, vring_align);
	vq->vq.callback = callback;
	vq->vq.vdev = vdev;
	vq->vq.name = name;
	vq->notify = notify;
	vq->broken = false;
	vq->last_used_idx = 0;
	vq->num_added = 0;
	list_add_tail(&vq->vq.list, &vdev->vqs);

    linfo("add virtqueue to vdev vqs list.\n");
    
#ifdef DEBUG
	vq->in_use = false;
#endif

//	vq->indirect = virtio_has_feature(vdev, VIRTIO_RING_F_INDIRECT_DESC);
//	vq->event = virtio_has_feature(vdev, VIRTIO_RING_F_EVENT_IDX);

	/* No callback?  Tell other side not to bother us. */
	if (!callback)
		vq->vring.avail->flags |= VRING_AVAIL_F_NO_INTERRUPT;

	/* Put everything in free lists. */
	vq->num_free = num;
	vq->free_head = 0;
	for (i = 0; i < num-1; i++) {
        linfo("vq->vring.desc[%2d].next = %d;\n", i, i+1);
		vq->vring.desc[i].next = i+1;
		vq->data[i] = NULL;
	}
	vq->data[i] = NULL;

	return &vq->vq;
}

int virtqueue_add_buf(struct virtqueue *_vq,
			  struct iovec sg[],
			  unsigned int out,
			  unsigned int in,
			  void *data)
{
    linfo("add buf to virtqueue.\n");
	struct vring_virtqueue *vq = to_vvq(_vq);
	unsigned int i, avail, prev;
	int head;

	START_USE(vq);

	assert(out + in < vq->vring.num);
	assert(out + in != 0);

	if (vq->num_free < out + in) {
		printf("Can't add buf len %i - avail = %i\n",
			 out + in, vq->num_free);
		/* FIXME: for historical reasons, we force a notify here if
		 * there are outgoing parts to the buffer.  Presumably the
		 * host should service the ring ASAP. */
		if (out)
			vq->notify(&vq->vq);
		END_USE(vq);
		return -ENOSPC;
	}

	/* We're about to use some buffers from the free list. */
	vq->num_free -= out + in;

	head = vq->free_head;
	for (i = vq->free_head; out; i = vq->vring.desc[i].next, out--) {
		vq->vring.desc[i].flags = VRING_DESC_F_NEXT;
		vq->vring.desc[i].addr = (unsigned long)sg->iov_base;
		vq->vring.desc[i].len = sg->iov_len;
        linfo("out-> add iov to vring.desc[%2d]. flags %d, addr 0x%p, len %d\n",
                i,
                vq->vring.desc[i].flags,
                vq->vring.desc[i].addr,
                vq->vring.desc[i].len);
		prev = i;
		sg++;
	}
	for (; in; i = vq->vring.desc[i].next, in--) {
		vq->vring.desc[i].flags = VRING_DESC_F_NEXT|VRING_DESC_F_WRITE;
		vq->vring.desc[i].addr = (unsigned long)sg->iov_base;
		vq->vring.desc[i].len = sg->iov_len;
        linfo(" in-> add iov to vring.desc[%2d]. flags %d, addr 0x%p, len %d\n",
                i,
                vq->vring.desc[i].flags,
                vq->vring.desc[i].addr,
                vq->vring.desc[i].len);
		prev = i;
		sg++;
	}
	/* Last one doesn't continue. */
	vq->vring.desc[prev].flags &= ~VRING_DESC_F_NEXT;

	/* Update free pointer */
	vq->free_head = i;

add_head:
	/* Set token. */
	vq->data[head] = data;

	/* Put entry in available array (but don't update avail->idx until they
	 * do sync).  FIXME: avoid modulus here? */
	avail = (vq->vring.avail->idx + vq->num_added++) % vq->vring.num;
	vq->vring.avail->ring[avail] = head;

	linfo("Added buffer head %i to %p\n", head, vq);
	END_USE(vq);

	return vq->num_free;
}

void virtqueue_kick(struct virtqueue *_vq)
{
    linfo("virtqueue kick.\n");
	struct vring_virtqueue *vq = to_vvq(_vq);
	u16 new, old;
	START_USE(vq);
	/* Descriptors and available array need to be set before we expose the
	 * new available array entries. */
	mb();

	old = vq->vring.avail->idx;
	new = vq->vring.avail->idx = old + vq->num_added;
	vq->num_added = 0;

	/* Need to update avail index before checking if we should notify */
	mb();

	if (vq->event ?
	    vring_need_event(vring_avail_event(&vq->vring), new, old) :
	    !(vq->vring.used->flags & VRING_USED_F_NO_NOTIFY)) {}
		/* Prod other side to tell it about changes. */
		//vq->notify(&vq->vq);

	END_USE(vq);
}
