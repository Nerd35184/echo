#ifndef _TASK_H_
#define _TASK_H_
#include "list.h"

typedef struct Task
{
    void *data;
    void (*handler)(void *data);
} Task_t;

extern_list(Task_t);

Task_t *alloc_task(void *data, void (*handler)(void *data), void (*clean)(void *data));

void free_task(void *data);

#endif