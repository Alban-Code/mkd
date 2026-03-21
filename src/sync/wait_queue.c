#include "internal/wait_queue.h"
#include "internal/task_internal.h"
#include "internal/kernel_internal.h"

#include <stdlib.h>

int wq_add(k_task_t *task, wait_queue_t *wq)
{
    if (task == NULL || wq == NULL)
        return -1;

    if (wq->head == NULL)
    {
        wq->head = task;
        wq->tail = task;
    }
    else
    {
        // Update the tail
        wq->tail->wq_next = task;
        wq->tail = task;
    }

    task_set_state(task, BLOCKED);

    return 0;
}

int wq_wake_one(wait_queue_t *wq)
{
    if (wq == NULL)
        return -1;

    k_task_t *t = wq->head;

    if (t == NULL)
        return -1;

    if (t == wq->tail)
    {
        wq->tail = NULL;
    }

    wq->head = t->wq_next;
    t->wq_next = NULL;

    if (g_kernel.head == NULL)
    {
        g_kernel.head = t;
        g_kernel.tail = t;
    }
    else
    {
        g_kernel.tail->next = t;
        g_kernel.tail = t;
    }

    task_set_state(t, READY);

    return 0;
}

int wq_count(wait_queue_t *wq)
{
    if (wq == NULL)
        return 0;
    k_task_t *it = wq->head;

    int counter = 0;
    while (it != NULL)
    {
        counter++;
        it = it->wq_next;
    }

    return counter;
}