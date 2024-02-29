#include "worker.h"
#include "util.h"
#define MAX_EVENTS 16

def_list_func(WorkHandler_t);
def_list_func(Worker_t);
def_list_func(WorkerPrt_t);

int wake_up(Worker_t *work_p)
{
    int ret = 0;
    if (work_p == NULL)
    {
        ret = -1;
        goto end;
    }
    printf("wake_up %d\n", work_p->wake_up_fd);
    uint64_t one = 1;
    ssize_t n = write(work_p->wake_up_fd, &one, sizeof one);
    if (n != sizeof(one))
    {
        printf("wake_up err,try write:%ld,actually\n", n);
    }
end:
    return ret;
}

typedef struct HandleWakeUpData
{
    int fd;
} HandleWakeUpData_t;

HandleWakeUpData_t *alloc_handle_wake_up_data(int fd)
{
    HandleWakeUpData_t *data;
    MALLOC(data, 1, sizeof(HandleWakeUpData_t));
    data->fd = fd;
    return data;
}

void free_handle_wake_up_data(HandleWakeUpData_t **data)
{
    FREE_PP(data, empty_destructor);
}

void handle_wake_up(HandleWakeUpData_t *data)
{
    if (data == NULL)
    {
        goto end;
    }
    printf("handle_wake_up %d\n", data->fd);
    uint64_t one = 1;
    ssize_t n = read(data->fd, &one, sizeof one);
    if (n != sizeof(one))
    {
        printf("handle_wake_up err,try write:%ld,actually\n", n);
    }
end:
    return;
}

void destruct_work_handler(WorkHandler_t *worker_handler_p)
{
    if (worker_handler_p->clean != NULL)
    {
        worker_handler_p->clean(&(worker_handler_p->data));
    }
}

void free_work_handler(WorkHandler_t **work_handler_pp)
{
    FREE_PP(work_handler_pp, destruct_work_handler);
}

int init_worker(Worker_t *work_p)
{
    int ret = 0;
    memset(work_p, 0, sizeof(Worker_t));
    work_p->epoll_fd = -1;
    work_p->wake_up_fd = -1;
    ret = pthread_mutex_init(&(work_p->lock), NULL);
    if (ret != 0)
    {
        goto end;
    }
    work_p->tasks = alloc_list(Task_t)();
    work_p->handlers = alloc_list(WorkHandler_t)();

    work_p->epoll_fd = epoll_create1(0);
    if (work_p->epoll_fd < 0)
    {
        ret = work_p->epoll_fd;
        goto end;
    }
    work_p->wake_up_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (work_p->wake_up_fd < 0)
    {
        ret = work_p->wake_up_fd;
        goto end;
    }
    WorkHandler_t handler = {
        fd : work_p->wake_up_fd,
        Read : (void (*)(void *))handle_wake_up,
        data : alloc_handle_wake_up_data(work_p->wake_up_fd),
        clean : (void (*)(void **))free_handle_wake_up_data
    };
    handler.data = &handler;
    ret = push_back_list(WorkHandler_t)(work_p->handlers, handler, NULL, destruct_work_handler);
    if (ret != 0)
    {
        goto end;
    }
    ret = set_epoll_event(work_p->epoll_fd, EPOLL_CTL_ADD, work_p->wake_up_fd, EPOLLIN);
    if (ret < 0)
    {
        goto end;
    }
end:
    if (ret < 0)
    {
        destruct_worker(work_p);
    }
    return ret;
};

Worker_t *alloc_worker()
{
    Worker_t *worker_p;
    MALLOC(worker_p, 1, sizeof(Worker_t));
    int ret = init_worker(worker_p);
    if (ret != 0)
    {
        FREE_P(worker_p, destruct_worker);
    }
    return worker_p;
}

void destruct_worker(Worker_t *work_p)
{
    if (work_p == NULL)
    {
        goto end;
    }
    if (work_p->epoll_fd >= 0)
    {
        close(work_p->epoll_fd);
        work_p->epoll_fd = -1;
    }
    if (work_p->wake_up_fd >= 0)
    {
        close(work_p->wake_up_fd);
        work_p->wake_up_fd = -1;
    }
    free_list(Task_t)(&(work_p->tasks));
    free_list(WorkHandler_t)(&(work_p->handlers));
end:
    return;
}

void free_worker(Worker_t **work_pp)
{
    FREE_PP(work_pp, destruct_worker);
}

int add_worker_task(Worker_t *work_p, Task_t task)
{
    int ret = 0;
    pthread_mutex_lock(&(work_p->lock));
    ret = push_back_list(Task_t)(work_p->tasks, task, NULL, NULL);
    if (ret < 0)
    {
        goto end;
    }
end:
    pthread_mutex_unlock(&(work_p->lock));
    return ret;
}

int get_worker_task(Worker_t *work_p, Task_t *result)
{
    int ret = 0;
    pthread_mutex_lock(&(work_p->lock));
    ret = pop_front_list(Task_t)(work_p->tasks, result, NULL);
    pthread_mutex_unlock(&(work_p->lock));
    return ret;
}

/*todo*/
void *do_work_loop(Worker_t *work_p)
{
    if (work_p == NULL)
    {
        goto end;
    }
    struct epoll_event events[MAX_EVENTS];
    int nfds = 0;
    while (!(work_p->stop))
    {
        nfds = epoll_wait(work_p->epoll_fd, events, MAX_EVENTS, 1000);
        for (int i = 0; i < nfds; ++i)
        {
            /*todo这里本来应该是个Map,或者封装一个查询方法*/
            ListNode(WorkHandler_t) *root_list_node_p = &(work_p->handlers->root);
            ListNode(WorkHandler_t) *next = NULL;
            int found = 0;
            int activated_fd = events[i].data.fd;
            for (ListNode(WorkHandler_t) *cur = root_list_node_p->next; cur != root_list_node_p && cur != NULL; cur = next)
            {
                next = cur->next;
                if (cur->data.fd != activated_fd)
                {
                    continue;
                }
                found = 1;
                Task_t new_task = {
                    data : cur->data.data,
                    handler : (void (*)(void *))cur->data.Read,
                };
                int ret = add_worker_task(work_p, new_task);
                if (ret != 0)
                {
                    printf("do_work_loop push_back_list err\n");
                }
            }
            if (!found)
            {
                /*todo这里应该要将描述符给剔除*/
                printf("call back not found:%d\n", activated_fd);
            }
        }
        Task_t task;
        while (get_worker_task(work_p, &task) == 0)
        {
            task.handler(task.data);
        }
    }
end:
    return NULL;
}
