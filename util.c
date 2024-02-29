#include "util.h"

void set_sock_nonblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        printf("fcntl(sock, GETFL)");
        exit(-1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        printf("fcntl(sock, SETFL, opts)");
        exit(-1);
    }
}

void empty_destructor(void *p) {}

int set_epoll_event(int epoll_fd, int op, int concern_fd, enum EPOLL_EVENTS event)
{
    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = concern_fd;
    return epoll_ctl(epoll_fd, op, concern_fd, &ev);
}
