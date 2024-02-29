#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "worker.h"
#include "util.h"

#define WORKER_COUNT (1)
#define LISTE_QUEUE_MAX (20)
#define READ_BUFF_SIZE (1024)

typedef struct Server
{
    /*todo这里应该搞成动态伸缩的*/
    Worker_t workers[WORKER_COUNT];
    int next_worker_index;
    int listen_fd;
    int port;
    pthread_mutex_t lock;
} Server_t;

void init_sockaddr_in(struct sockaddr_in *server_address_p, int port)
{
    memset(server_address_p, 0, sizeof(struct sockaddr_in));
    server_address_p->sin_family = AF_INET;
    inet_aton(LOCAL_ADDRESS, &(server_address_p->sin_addr));
    server_address_p->sin_port = htons(port);
}

Worker_t *get_next_worker(Server_t *server_p)
{
    size_t worker_count = sizeof(server_p->workers) / sizeof(Worker_t);
    pthread_mutex_lock(&(server_p->lock));
    Worker_t *worker_p = &(server_p->workers[server_p->next_worker_index]);
    server_p->next_worker_index = (server_p->next_worker_index + 1) % worker_count;
    pthread_mutex_unlock(&(server_p->lock));
    return worker_p;
}

typedef struct HandleMessageData
{
    int socket_fd;
    int epoll_fd;
} HandleMessageData_t;

HandleMessageData_t *alloc_handle_message_data(int socket_fd, int epoll_fd)
{
    HandleMessageData_t *data;
    MALLOC(data, 1, sizeof(HandleMessageData_t));
    data->socket_fd = socket_fd;
    data->epoll_fd = epoll_fd;
    return data;
}

void free_handle_message_data(HandleMessageData_t **data)
{
    FREE_PP(data, empty_destructor);
}

void handle_message(HandleMessageData_t *data)
{
    if (data == NULL)
    {
        printf("handle_message err,data is null");
        goto end;
    }
    char buf[READ_BUFF_SIZE];
    memset(buf, 0, sizeof(buf));
    int n = read(data->socket_fd, buf, READ_BUFF_SIZE);
    if (n == 0)
    {
        set_epoll_event(data->epoll_fd, EPOLL_CTL_DEL, data->socket_fd, 0);
        goto end;
    }
    write(data->socket_fd, buf, n);
end:
    return;
}

typedef struct RegisteredHandleMessageHandlerData
{
    Worker_t *worker_p;
    int client_sock;
} RegisteredHandleMessageHandlerData_t;

RegisteredHandleMessageHandlerData_t *alloc_registered_handle_message_handler_data(Worker_t *worker_p, int client_sock)
{
    RegisteredHandleMessageHandlerData_t *data;
    MALLOC(data, 1, sizeof(RegisteredHandleMessageHandlerData_t));
    data->client_sock = client_sock;
    data->worker_p = worker_p;
    return data;
}

void free_registered_handle_message_handler_data(RegisteredHandleMessageHandlerData_t **data)
{
    FREE_PP(data, empty_destructor);
}

void registered_handle_message_handler(RegisteredHandleMessageHandlerData_t *data)
{
    int ret = 0;
    Worker_t *worker_p = data->worker_p;
    set_sock_nonblocking(data->client_sock);
    WorkHandler_t handler = {
        fd : data->client_sock,
        Read : (void (*)(void *))handle_message,
        data : alloc_handle_message_data(data->client_sock, worker_p->epoll_fd),
        clean : (void (*)(void **))free_handle_message_data
    };
    ret = set_epoll_event(worker_p->epoll_fd, EPOLL_CTL_ADD, data->client_sock, EPOLLIN);
    if (ret != 0)
    {
        exit(-1);
    }
    ret = push_back_list(WorkHandler_t)(worker_p->handlers, handler, NULL, destruct_work_handler);
    if (ret != 0)
    {
        exit(-1);
    }
    free_registered_handle_message_handler_data(&data);
}

typedef struct HandleAcceptData
{
    int fd;
    Server_t *server_p;
} HandleAcceptData_t;

HandleAcceptData_t *alloc_handle_accept_data(int fd, Server_t *server_p)
{
    HandleAcceptData_t *data;
    MALLOC(data, 1, sizeof(HandleAcceptData_t));
    data->fd = fd;
    data->server_p = server_p;
    return data;
}

void free_handle_accept_data(HandleAcceptData_t **data)
{
    FREE_PP(data, empty_destructor);
}

void handle_accept(HandleAcceptData_t *data)
{
    if (data == NULL)
    {
        printf("handle_accept err,data is null");
        goto end;
    }
    int fd = data->fd;
    Server_t *server_p = data->server_p;
    struct sockaddr_in client_address;
    socklen_t client_sock_len = sizeof(struct sockaddr_in);
    int client_sock = accept(fd, (struct sockaddr *)&client_address, &client_sock_len);
    if (client_sock == -1)
    {
        printf("handle_accept err\n");
        goto end;
    }
    Worker_t *worker_p = get_next_worker(server_p);
    Task_t task = {
        data : alloc_registered_handle_message_handler_data(worker_p, client_sock),
        handler : (void (*)(void *))registered_handle_message_handler,
    };
    add_worker_task(worker_p, task);
end:
    return;
}

typedef struct RegisteredHandleAcceptTaskData
{
    int fd;
    Worker_t *worker_p;
    Server_t *server_p;
} RegisteredHandleAcceptTaskData_t;

void registered_handle_accept(RegisteredHandleAcceptTaskData_t *RegisteredHandleAcceptTaskData_p)
{
    int fd = RegisteredHandleAcceptTaskData_p->fd;
    Worker_t *worker_p = RegisteredHandleAcceptTaskData_p->worker_p;
    Server_t *server_p = RegisteredHandleAcceptTaskData_p->server_p;
    WorkHandler_t handler = {
        fd : fd,
        Read : (void (*)(void *))handle_accept,
        data : alloc_handle_accept_data(fd, server_p),
        clean : (void (*)(void **))free_handle_accept_data
    };
    int ret = push_back_list(WorkHandler_t)(worker_p->handlers, handler, NULL, destruct_work_handler);
    if (ret != 0)
    {
        printf("registered_handle_accept err\n");
        exit(-1);
    }
    ret = set_epoll_event(worker_p->epoll_fd, EPOLL_CTL_ADD, fd, EPOLLIN);
    if (ret != 0)
    {
        printf("set_epoll_event err\n");
        exit(-1);
    }
    FREE_P(RegisteredHandleAcceptTaskData_p, empty_destructor);
    return;
}

int init_server(Server_t *server_p, int port)
{
    int ret = 0;
    if (server_p == NULL)
    {
        ret = -1;
        printf("init_server err,server_p is null\n");
        goto end;
    }
    memset(server_p, 0, sizeof(Server_t));
    pthread_mutex_init(&(server_p->lock), NULL);
    size_t worker_count = sizeof(server_p->workers) / sizeof(Worker_t);
    for (int i = 0; i < worker_count; ++i)
    {
        Worker_t *work_p = &(server_p->workers[i]);
        ret = init_worker(work_p);
        if (ret != 0)
        {
            printf("init_server init_worker err %d %s\n", ret, strerror(ret));
            exit(-1);
        }
        pthread_t thread;
        ret = pthread_create(&thread, NULL, (void *(*)(void *))do_work_loop, work_p);
        if (ret != 0)
        {
            printf("init_server pthread_create err %d %s\n", ret, strerror(ret));
            exit(-1);
        }
        work_p->thread_id = thread;
    }
    server_p->port = port;
    server_p->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_p->listen_fd == -1)
    {
        printf("socket faild\n");
        goto end;
    }
    set_sock_nonblocking(server_p->listen_fd);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    init_sockaddr_in(&server_addr, server_p->port);
    ret = bind(server_p->listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        printf("bind faild\n");
        goto end;
    }
    ret = listen(server_p->listen_fd, LISTE_QUEUE_MAX);
    if (ret < 0)
    {
        printf("listen faild\n");
        goto end;
    }
    Worker_t *worker_p = get_next_worker(server_p);
    RegisteredHandleAcceptTaskData_t *task_data;
    MALLOC(task_data, 1, sizeof(RegisteredHandleAcceptTaskData_t));
    task_data->fd = server_p->listen_fd;
    task_data->worker_p = worker_p;
    task_data->server_p = server_p;
    Task_t task = {
        data : task_data,
        handler : (void (*)(void *))registered_handle_accept,
    };
    ret = add_worker_task(worker_p, task);
    if (ret < 0)
    {
        printf("add_worker_task faild\n");
        goto end;
    }
end:
    return ret;
}

void destruct_server(Server_t *server_p)
{
    if (server_p == NULL)
    {
        goto end;
    }
    size_t worker_count = sizeof(server_p->workers) / sizeof(Worker_t);
    for (int i = 0; i < worker_count; ++i)
    {
        server_p->workers[i].stop = TRUE;
    }
    for (int i = 0; i < worker_count; ++i)
    {
        void **thread_return = NULL;
        pthread_join(server_p->workers[i].thread_id, thread_return);
    }
    for (int i = 0; i < worker_count; ++i)
    {
        destruct_worker(&(server_p->workers[i]));
    }
    printf("destruct_server before close\n");
    close(server_p->listen_fd);
end:
    return;
}

int main()
{
    int ret = 0;
    int port = SERVER_PORT;
    Server_t server;
    ret = init_server(&server, port);
    if (ret != 0)
    {
        goto end;
    }
    for (;;)
    {
        sleep(5);
    }
end:
    return ret;
}