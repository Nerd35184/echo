#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#define SERVER_PORT (8888)
#define LOCAL_ADDRESS ("0.0.0.0")

#define FREE_P(p, destructor) \
    if (destructor != NULL)   \
    {                         \
        destructor(p);        \
    }                         \
    free(p);                  \
    p = NULL;

#define FREE_PP(pp, destructor)        \
    do                                 \
    {                                  \
        if (pp == NULL || *pp == NULL) \
        {                              \
            break;                     \
        }                              \
        if (destructor != NULL)        \
        {                              \
            destructor(*pp);           \
        }                              \
        *pp = NULL;                    \
    } while (0)

#define MALLOC(ptr, count, size)                \
    ptr = malloc(count * size);                 \
    if (ptr == NULL)                            \
    {                                           \
        printf("malloc %s\n", strerror(errno)); \
        exit(-1);                               \
    }

#define MALLOC_INIT(ptr, count, size) \
    MALLOC(ptr, count, size);         \
    memset(ptr, 0, count *size)

#define REALLOC(ptr, count, size)               \
    ptr = realloc(ptr, count * size);           \
    if (ptr == NULL)                            \
    {                                           \
        printf("malloc %s\n", strerror(errno)); \
        exit(-1);                               \
    }

#endif

void empty_destructor(void *);
void set_sock_nonblocking(int sock);
int set_epoll_event(int epoll_fd, int concern_fd, int op, uint32_t event);

#define DEBUG_PRINT printf
#define DEBUG_SLEEP sleep