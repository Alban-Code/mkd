#ifndef KERNEL_INTERNAL_H
#define KERNEL_INTERNAL_H

#include <stdbool.h>

#include "task_internal.h"

typedef struct
{
    bool initialized;
    int next_id;
    // Pointeur vers le début de la liste de tâches à run
    k_task_t *head;
    // Pointeur vers la fin de la liste de tâches à run
    k_task_t *tail;
    // Pointeur vers la tâche courante
    k_task_t *current_task;
    // Contexte de main
    ucontext_t main_ctx;
    // pointeur vers la tâche à free si elle est terminée
    k_task_t *to_free;
} kernel_t;

extern kernel_t g_kernel;

#endif