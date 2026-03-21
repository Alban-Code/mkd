#ifndef TASK_INTERNAL_H
#define TASK_INTERNAL_H

#include <ucontext.h>
#include "kernel/task.h"

typedef struct task
{
    // Contexte de la tâche
    ucontext_t ctx;
    // Stack de la tâche avec sa taille
    char *stack;
    size_t stack_size;
    // Etat de la tâche
    task_state_t state;
    // Fonction de la tâche et ses arguments
    task_fn_t fn;
    void *args;
    // Id de la tâche
    int id;
    // Pointeur vers la prochaine task dans la liste
    struct task *next;
    // Pointeur vers la prochaine task bloquée
    struct task *wq_next;
} k_task_t;

void task_set_state(k_task_t *task, task_state_t new_state);

k_task_t *get_current_task();

#endif