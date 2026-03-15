#ifndef TASK_H
#define TASK_H

typedef enum
{
    NEW,
    READY,
    RUNNING,
    TERMINATED,
    BLOCKED
} task_state_t;

typedef void (*task_fn_t)(void *);

void k_yield();

int k_task_create(task_fn_t fn, void *args);

void k_run();

int k_task_count_in_state(task_state_t state);

task_state_t k_current_task_state(void);
#endif