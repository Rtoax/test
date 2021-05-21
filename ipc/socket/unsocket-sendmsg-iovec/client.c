#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "common.h"

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

    if (argc != 3) {
        fprintf (stderr, "Usage: %s <file-name1> <file-name2>\n", argv[0]);
        exit (1);
    }

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error ("Failed to create socket");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNSOCKET_PATH, sizeof(addr.sun_path)-1);

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

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        handle_error ("Failed to connect to socket");

    send_fd (sfd, fds, 2);

    if (close(fds[0]) == -1)
        handle_error ("Failed to close fd0");

    if (close(fds[1]) == -1)
        handle_error ("Failed to close fd1");


    exit(EXIT_SUCCESS);
}

