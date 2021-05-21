#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include "common.h"

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while(0)

static int * recv_fd(int socket, int n) 
{
    int *fds = malloc (n * sizeof(int));
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;

    char buf[CMSG_SPACE(n * sizeof(int))], dup[256];
    memset(buf, '\0', sizeof(buf));

    struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    if (recvmsg (socket, &msg, 0) < 0)
        handle_error ("Failed to receive message");

    cmsg = CMSG_FIRSTHDR(&msg);

    memcpy (fds, (int *) CMSG_DATA(cmsg), n * sizeof(int));

    return fds;
}

int main(int argc, char *argv[]) 
{
    ssize_t nbytes;
    char buffer[256];
    int sfd, cfd, *fds;
    struct sockaddr_un addr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        handle_error ("Failed to create socket");

    if (unlink (UNSOCKET_PATH) == -1 && errno != ENOENT)
        handle_error ("Removing socket file failed");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, UNSOCKET_PATH, sizeof(addr.sun_path)-1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        handle_error ("Failed to bind to socket");

    if (listen(sfd, 5) == -1)
        handle_error ("Failed to listen on socket");

    cfd = accept(sfd, NULL, NULL);
    if (cfd == -1)
        handle_error ("Failed to accept incoming connection");

    fds = recv_fd(cfd, 2);

    for (int i=0; i<2; ++i) {
        
        fprintf (stdout, "Reading from passed fd %d\n", fds[i]);
        while ((nbytes = read(fds[i], buffer, sizeof(buffer))) > 0)
            write(1, buffer, nbytes);
        *buffer = '\0';
    }

    if (close(fds[0]) == -1)
        handle_error ("Failed to close fd0");

    if (close(fds[1]) == -1)
        handle_error ("Failed to close fd1");

    if (close(cfd) == -1)
        handle_error ("Failed to close client socket");

    return 0;
}

