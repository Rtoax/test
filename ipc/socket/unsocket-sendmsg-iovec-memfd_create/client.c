#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while(0)

static void send_fd(int socket, int *fds, int n)  // send fd by socket
{
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(n * sizeof(int))], dup[256];
    memset(buf, '\0', sizeof(buf));
    struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };


    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;   /* Transfer file descriptors.  */
    cmsg->cmsg_len = CMSG_LEN(n * sizeof(int));

    memcpy ((int *) CMSG_DATA(cmsg), fds, n * sizeof (int));

    if (sendmsg (socket, &msg, 0) < 0)
        handle_error ("Failed to send message");
}

int main(int argc, char *argv[]) 
{
    int sfd, fds[2];
    struct sockaddr_un addr;

#if 0
    if (argc != 3) {
        fprintf (stderr, "Usage: %s <file-name1> <file-name2>\n", argv[0]);
        exit (1);
    }
#endif

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error ("Failed to create socket");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/tmp/fd-pass.socket", sizeof(addr.sun_path)-1);

#if 0
    fds[0] = open(argv[1], O_RDONLY);
    if (fds[0] < 0)
        handle_error ("Failed to open file 1 for reading");
    else
        fprintf (stdout, "Opened fd %d in parent\n", fds[0]);

    fds[1] = open(argv[2], O_RDONLY);
    if (fds[1] < 0)
        handle_error ("Failed to open file 2 for reading");
    else
        fprintf (stdout, "Opened fd %d in parent\n", fds[1]);
#else
//    0x400000 - 4M
#define SIZE	0xff
//	fds[0] = memfd_create("shma", 0);
	fds[0] = syscall(SYS_memfd_create, "shma", 0);
	if(fds[0] < 0){
		handle_error ("SYS_memfd_create error.");
	} else {
		printf("Opened fd %d in parent\n", fds[0]);
	}
	ftruncate(fds[0], SIZE);
	void *ptr0 = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fds[0], 0);
	memset(ptr0, 'A', SIZE);
	munmap(ptr0, SIZE);

    
//	fds[1] = memfd_create("shmb", 0);
	fds[1] = syscall(SYS_memfd_create, "shmb", 0);
	if(fds[1] < 0){
		handle_error ("SYS_memfd_create error.");
	} else {
		printf("Opened fd %d in parent\n", fds[1]);
	}
	ftruncate(fds[1], SIZE);
	void *ptr1 = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fds[1], 0);
	memset(ptr1, 'B', SIZE);
	munmap(ptr1, SIZE);

#endif
    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        handle_error ("Failed to connect to socket");

    send_fd (sfd, fds, 2);

    exit(EXIT_SUCCESS);
}

