#include "worker.h"

void *wake_up_loop(Worker_t *work_p)
{
    for (;;)
    {
        wake_up(work_p);
        sleep(5);
    }
    return NULL;
}

int main()
{
    int ret = 0;
    Worker_t *work_p = alloc_worker();
    if (work_p == NULL)
    {
        printf("alloc_worker err\n");
        goto end;
    }
    pthread_t thread1;
    void **thread_result = NULL;
    ret = pthread_create(&thread1, NULL, (void *(*)(void *))do_work_loop, work_p);
    if (ret != 0)
    {
        goto end;
    }
    pthread_t thread2;
    ret = pthread_create(&thread2, NULL, (void *(*)(void *))wake_up_loop, work_p);
    if (ret != 0)
    {
        goto end;
    }
end:
    pthread_join(thread1, thread_result);
    pthread_join(thread2, thread_result);
    free_worker(&work_p);
    return 0;
}