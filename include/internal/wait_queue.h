#ifndef WAIT_QUEUE_H
#define WAIT_QUEUE_H

#include "task_internal.h"

typedef struct wait_queue
{
    k_task_t *head;
    k_task_t *tail;
} wait_queue_t;

int wq_add(k_task_t *task, wait_queue_t *wq);

int wq_wake_one(wait_queue_t *wq);

int wq_count(wait_queue_t *wq);
#endif