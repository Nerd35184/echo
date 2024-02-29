#ifndef __WORKER_H__
#define __WORKER_H__
#include "list.h"
#include "task.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <pthread.h>

typedef struct Worker Worker_t;

/*todo 其实还应该实现可写事件，错误事件等*/
typedef struct WorkHandler
{
    int fd;
    void *data;
    void (*clean)(void **data);
    void (*Read)(void *data);
} WorkHandler_t;

extern_list(WorkHandler_t);

void free_work_handler(WorkHandler_t **work_handler_pp);
void destruct_work_handler(WorkHandler_t *worker_handler_p);

struct Worker
{
    pthread_t thread_id;
    int epoll_fd;
    int wake_up_fd;
    int stop;
    List(Task_t) * tasks;
    /*todo这里其实应该用map来存，key为fd，value为回调函数，不过对于echo而言，由于工作量的问题用list也无伤大雅*/
    List(WorkHandler_t) * handlers;
    /*只控制tasks，handlers不允许多线程调度*/
    pthread_mutex_t lock;
};

typedef Worker_t *WorkerPrt_t;

extern_list(Worker_t);
extern_list(WorkerPrt_t);

int init_worker(Worker_t *work_p);
Worker_t *alloc_worker();
void free_worker(Worker_t **work_pp);
void destruct_worker(Worker_t *work_pp);
void destruct_worker(Worker_t *work_p);
void *do_work_loop(Worker_t *work_p);
int wake_up(Worker_t *work_p);
Task_t get_handle_task();
int add_worker_task(Worker_t *work_p, Task_t task);

#endif