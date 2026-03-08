#include <stdlib.h>
#include <ucontext.h>

#include "kernel/task.h"
#include "internal/task_internal.h"
#include "internal/kernel_internal.h"

#define TASK_STACK_SIZE (64 * 1024)

static int next_id = 0;

static void task_trampoline(void)
{
    k_task_t *task = g_kernel.current_task;
    task->fn(task->args);
    task->state = TERMINATED;
    k_yield();
}

int k_task_create(task_fn_t fn, void *args)
{
    // Création d'une task
    k_task_t *task = malloc(sizeof(k_task_t));
    if (task == NULL)
    {
        return -1;
    }

    task->stack = malloc(TASK_STACK_SIZE);
    if (task->stack == NULL)
    {
        free(task);
        return -1;
    }

    task->stack_size = TASK_STACK_SIZE;

    task->state = NEW;
    task->fn = fn;
    task->args = args;
    task->id = next_id++;
    task->next = NULL;

    getcontext(&task->ctx);

    // On dit ou est la stack de ce context dans la mémoire
    task->ctx.uc_stack.ss_sp = task->stack;
    // On dit la taille de cette stack
    task->ctx.uc_stack.ss_size = task->stack_size;
    // Quand le context est terminé, à quel context revenir
    task->ctx.uc_link = &g_kernel.main_ctx;

    makecontext(&task->ctx, task_trampoline, 0);

    task->state = READY;

    if (g_kernel.head == NULL)
    {
        g_kernel.head = task;
        g_kernel.tail = task;
    }
    else
    {
        g_kernel.tail->next = task;
        g_kernel.tail = task;
    }

    return task->id;
}