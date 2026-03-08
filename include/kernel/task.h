#ifndef TASK_H
#define TASK_H

typedef void (*task_fn_t)(void *);

void k_yield();

int k_task_create(task_fn_t fn, void *args);

void k_run();

#endif