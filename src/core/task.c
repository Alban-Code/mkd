#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>

#include "kernel/task.h"
#include "internal/task_internal.h"
#include "internal/kernel_internal.h"

#define TASK_STACK_SIZE (64 * 1024)

static void task_trampoline(void)
{
    k_task_t *task = g_kernel.current_task;
    task->fn(task->args);
    task_set_state(task, TERMINATED);
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

    task_set_state(task, NEW);
    task->fn = fn;
    task->args = args;
    task->id = g_kernel.next_id++;
    task->next = NULL;

    getcontext(&task->ctx);

    // On dit ou est la stack de ce context dans la mémoire
    task->ctx.uc_stack.ss_sp = task->stack;
    // On dit la taille de cette stack
    task->ctx.uc_stack.ss_size = task->stack_size;
    // Quand le context est terminé, à quel context revenir
    task->ctx.uc_link = &g_kernel.main_ctx;

    makecontext(&task->ctx, task_trampoline, 0);

    task_set_state(task, READY);

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

void task_set_state(k_task_t *task, task_state_t new_state)
{
    switch (new_state)
    {
    case NEW:
        task->state = NEW;
        break;
    case READY:
        if (task->state != NEW && task->state != BLOCKED && task->state != RUNNING)
        {
            assert(false && "Cannot change state to READY if not in NEW, BLOCKED or RUNNING state");
        }
        task->state = READY;
        break;
    case RUNNING:
        if (task->state != READY)
        {
            assert(false && "Cannot change state to RUNNING if not in READY state");
        }
        task->state = RUNNING;
        break;
    case TERMINATED:
        if (task->state != RUNNING)
        {
            assert(false && "Cannot change state to TERMINATED if not in RUNNING state");
        }
        task->state = TERMINATED;
        break;
    case BLOCKED:
        if (task->state != RUNNING)
        {
            assert(false && "Cannot change state to BLOCKED if not in RUNNING state");
        }
        task->state = BLOCKED;
        break;
    default:
        break;
    }
}

int k_task_count_in_state(task_state_t state)
{
    if (state == RUNNING)
    {
        if (g_kernel.current_task != NULL)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        k_task_t *task_it = g_kernel.head;
        int counter = 0;
        while (task_it != NULL)
        {
            if (task_it->state == state)
            {
                counter++;
            }
            task_it = task_it->next;
        }
        return counter;
    }
}

task_state_t k_current_task_state(void)
{
    return g_kernel.current_task->state;
}

k_task_t *get_current_task()
{
    return g_kernel.current_task;
}