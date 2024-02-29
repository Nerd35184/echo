#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "util.h"
#include <pthread.h>

void *write_loop(int *socket)
{
    char buf[1024];
    int len = 0;
    ssize_t n = 0;
    for (;;)
    {
        memset(buf, 0, 1024);
        scanf("%1023s", buf);
        len = strlen(buf);
        n = write(*socket, buf, len);
        if (n == 0)
        {
            close(*socket);
            break;
        }
    }
    return NULL;
}

void *read_loop(int *socket)
{
    char buf[1024];
    int buf_size_minus1 = 1023;
    ssize_t n = 0;
    for (;;)
    {
        memset(buf, 0, 1024);
        n = read(*socket, buf, buf_size_minus1);
        if (n == 0)
        {
            close(*socket);
            break;
        }
        printf("%s\n", buf);
    }
    return NULL;
}

int main(void)
{
    int ret = 0;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0)
    {
        printf("socket err\n");
        exit(-1);
    }
    int server_socket = ret;
    ret = connect(server_socket, (struct sockaddr *)&server, sizeof server);
    if (ret < 0)
    {
        printf("connect err\n");
        exit(-1);
    }
    pthread_t thread1;
    ret = pthread_create(&thread1, NULL, (void *(*)(void *))write_loop, &server_socket);
    if (ret < 0)
    {
        exit(-1);
    }
    pthread_t thread2;
    ret = pthread_create(&thread2, NULL, (void *(*)(void *))read_loop, &server_socket);
    if (ret < 0)
    {
        printf("pthread_create err\n");
        exit(-1);
    }
    for (;;)
    {
        sleep(20);
    }

    return 0;
}