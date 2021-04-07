#include <stdio.h>
#include <stdlib.h>
#include <virtio.h>
#include <virtio_client.h>
#include <virtio_server.h>
#include <linux/list.h>
#include <utils.h>


#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>


struct virtio_device vdev = {0};
struct virtqueue *vq;
void *shm, *vq_ptr, *data_ptr;


void *enqueue_task(void*arg){
    
    struct iovec iov[2];
	iov[0] = (struct iovec) {
		.iov_base = data_ptr,
		.iov_len = 4,
	};
	iov[1] = (struct iovec) {
		.iov_base = data_ptr + 4,
		.iov_len = 6,
	};
//    while(1) {
	virtqueue_add_buf(vq, iov, 2, 0, NULL);
	virtqueue_kick(vq);
//        sleep(1);
//    }

}

void *dequeue_task(void*arg){
	u16 out, in, i;
    struct iovec iov_buf[64];
    
	if (virt_queue__available(to_vvq(vq)))
		linfo("Buffers available - OK!\n");

//    while(1) {
    out = in  = 0;
	virt_queue__get_iov(to_vvq(vq), iov_buf, &out, &in);

	linfo("Output: \n");
	for (i = 0; i < out; i++) {
		linfo("%.*s\n", (int)iov_buf[i].iov_len, (char*)iov_buf[i].iov_base);
		fflush(stdout);
	}
	linfo("\n");

//    }

}

int main()
{
    pthread_t enqueue;
    pthread_t dequeue;

	int fd = shm_open("virtio_shm", O_CREAT | O_RDWR, 0777);
	u16 out, in, i;

	ftruncate(fd, 131072);
	shm = mmap(NULL, 131072, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	vq_ptr = shm;
	data_ptr = shm + 32768;


	strncpy(data_ptr, "a1b2c3d4e5", 10);
	linfo("Input: %s\n", (char *)data_ptr);

	INIT_LIST_HEAD(&vdev.vqs);
    linfo("Init link list.\n");
    
	vq = vring_new_virtqueue(16, 4096, &vdev, vq_ptr, vq_ptr + 1024, NULL, NULL, "test");
	if (virt_queue__available(to_vvq(vq)))
		lerror("No buffers available - OK!\n");

    pthread_create(&enqueue, NULL, enqueue_task, NULL);
    pthread_create(&dequeue, NULL, dequeue_task, NULL);

	pthread_setname_np(enqueue, "enqueue");
	pthread_setname_np(dequeue, "dequeue");

	pthread_setname_np(pthread_self(), "test-1");

    pthread_join(enqueue, NULL);
    pthread_join(dequeue, NULL);
}

