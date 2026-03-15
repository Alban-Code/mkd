#include <stdlib.h>
#include <ucontext.h>

#include "kernel/task.h"
#include "internal/kernel_internal.h"
#include "kernel/log.h"

static k_task_t *scheduler_next()
{
    if (g_kernel.head == NULL)
    {
        return NULL;
    }

    k_task_t *cur = g_kernel.head;
    g_kernel.head = g_kernel.head->next;

    if (g_kernel.head == NULL)
    {
        g_kernel.tail = NULL;
    }
    cur->next = NULL;

    return cur;
}

void k_yield()
{
    if (g_kernel.to_free != NULL)
    {
        free(g_kernel.to_free->stack);
        free(g_kernel.to_free);
        g_kernel.to_free = NULL;
    }

    k_task_t *cur = g_kernel.current_task;
    k_task_t *next = scheduler_next();

    if (cur->state != TERMINATED)
    {
        if (next == NULL)
        {
            return;
        }
        task_set_state(cur, READY);

        if (g_kernel.head == NULL)
        {
            g_kernel.head = cur;
            g_kernel.tail = cur;
        }
        else
        {
            g_kernel.tail->next = cur;
            g_kernel.tail = cur;
        }

        g_kernel.current_task = next;
        task_set_state(next, RUNNING);
        swapcontext(&cur->ctx, &next->ctx);
    }
    else
    {
        g_kernel.to_free = cur;
        if (next == NULL)
        {
            setcontext(&g_kernel.main_ctx);
        }
        else
        {
            g_kernel.current_task = next;
            task_set_state(next, RUNNING);
            setcontext(&next->ctx);
        }
    }
}

void k_run()
{
    if (!g_kernel.initialized)
    {
        LOG(LOG_ERROR, "Kernel not initialized, cannot run any task");
        return;
    }

    if (g_kernel.head != NULL)
    {
        k_task_t *cur = scheduler_next();
        task_set_state(cur, RUNNING);
        g_kernel.current_task = cur;
        swapcontext(&g_kernel.main_ctx, &cur->ctx);
        if (g_kernel.to_free != NULL)
        {
            free(g_kernel.to_free->stack);
            free(g_kernel.to_free);
            g_kernel.to_free = NULL;
        }
    }

    return;
}