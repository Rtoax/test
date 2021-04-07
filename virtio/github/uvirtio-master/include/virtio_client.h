#ifndef VIRTIO_CLIENT_H_
#define VIRTIO_CLIENT_H_

#include <linux/virtio_ring.h>

#include <linux/types.h>
#include <sys/uio.h>
#include <stdbool.h>

#include "virtio_server.h"

static inline u16 virt_queue__pop(struct vring_virtqueue *vq)
{
	return vq->vring.avail->ring[vq->last_avail_idx++ % vq->vring.num];
}

static inline struct vring_desc *virt_queue__get_desc(struct vring_virtqueue *vq, u16 desc_ndx)
{
	return &vq->vring.desc[desc_ndx];
}

static inline bool virt_queue__available(struct vring_virtqueue *vq)
{
	if (!vq->vring.avail)
		return 0;
	return vq->vring.avail->idx !=  vq->last_avail_idx;
}

struct vring_used_elem *virt_queue__set_used_elem(struct vring_virtqueue *vq, u32 head, u32 len);

u16 virt_queue__get_iov(struct vring_virtqueue *vq, struct iovec iov[], u16 *out, u16 *in);
u16 virt_queue__get_inout_iov(struct vring_virtqueue *vq, struct iovec in_iov[],
				struct iovec out_iov[], u16 *in, u16 *out);
int virtio__get_dev_specific_field(int offset, bool msix, bool features_hi, u32 *config_off);

#endif