#ifndef VIRTIO_H_
#define VIRTIO_H_

#include <linux/virtio_ring.h>
#include <stdbool.h>

#define mb() asm volatile ("" : : : "memory")

/**
 * virtqueue - a queue to register buffers for sending or receiving.
 * @list: the chain of virtqueues for this device
 * @callback: the function to call when buffers are consumed (can be NULL).
 * @name: the name of this virtqueue (mainly for debugging)
 * @vdev: the virtio device this queue was created for.
 * @priv: a pointer for the virtqueue implementation to use.
 */
struct virtqueue {
	struct list_head list;
	void (*callback)(struct virtqueue *vq);
	const char *name;
	struct virtio_device *vdev;
	void *priv;
};

/**
 * virtio_device - representation of a device using virtio
 * @index: unique position on the virtio bus
 * @dev: underlying device.
 * @id: the device type identification (used to match it with a driver).
 * @config: the configuration ops for this device.
 * @vqs: the list of virtqueues for this device.
 * @features: the features supported by both driver and device.
 * @priv: private pointer for the driver's use.
 */
struct virtio_device {
	int index;
	struct virtio_config_ops *config;
	struct list_head vqs;
	/* Note that this is a Linux set_bit-style bitmap. */
	unsigned long features[1];
	void *priv;
};

struct vring_virtqueue
{
	struct virtqueue vq;

	/* Actual memory layout for this queue */
	struct vring vring;

	/* Other side has made a mess, don't try any more. */
	bool broken;

	/* Host supports indirect buffers */
	bool indirect;

	/* Host publishes avail event idx */
	bool event;

	/* Number of free buffers */
	unsigned int num_free;
	/* Head of free buffer list. */
	unsigned int free_head;
	/* Number we've added since last sync. */
	unsigned int num_added;

	/* Last used index we've seen. */
	u16 last_used_idx;

	/* Last available index we've seen. */
	u16 last_avail_idx;

	/* How to notify other side. FIXME: commonalize hcalls! */
	void (*notify)(struct virtqueue *vq);

#ifdef DEBUG
	/* They're supposed to lock for us. */
	unsigned int in_use;
#endif

	/* Tokens for callbacks. */
	void *data[];
};

#endif