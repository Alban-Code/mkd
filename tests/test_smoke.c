#include <assert.h>
#include <stdio.h>

#include "kernel/kernel.h"
#include "kernel/task.h"

typedef struct
{
    int *order;
    int *step;
} test_ctx_t;

static void fn_task1(void *args)
{
    test_ctx_t *ctx = args;
    ctx->order[*ctx->step] = 0;
    *ctx->step = *ctx->step + 1;
    k_yield();
    ctx->order[*ctx->step] = 0;
    *ctx->step = *ctx->step + 1;
}

static void fn_task2(void *args)
{
    test_ctx_t *ctx = args;
    ctx->order[*ctx->step] = 1;
    *ctx->step = *ctx->step + 1;
    k_yield();
    ctx->order[*ctx->step] = 1;
    *ctx->step = *ctx->step + 1;
}

/* Test 1 : alternance correcte entre 2 tasks */
static int test_alternance()
{
    int order[4];
    int step = 0;
    k_init();

    test_ctx_t ctx = {order, &step};

    k_task_create(fn_task1, &ctx);
    k_task_create(fn_task2, &ctx);
    k_run();
    for (int i = 0; i < 4; i++)
    {
        if (!(order[i] == i % 2))
        {
            k_shutdown();
            return -1;
        }
    }

    k_shutdown();
    return 0;
}

static void fn_task3(void *args)
{
    task_state_t *observed = args;
    *observed = k_current_task_state();
}

/* Test 2 : transitions d'état */
static int test_etats()
{
    task_state_t observed;
    k_init();
    k_task_create(fn_task3, &observed);
    assert(k_task_count_in_state(READY) == 1);
    k_run();
    assert(observed == RUNNING);
    assert(k_task_count_in_state(READY) == 0);
    k_shutdown();

    return 0;
}

static int invariant_violated = 0;

static void fn_check_invariant(void *arg)
{
    (void)arg;

    // La task courante doit être RUNNING
    if (k_current_task_state() != RUNNING)
        invariant_violated = 1;

    // Une seule task RUNNING à la fois (aucune dans la queue)
    if (k_task_count_in_state(RUNNING) != 1)
        invariant_violated = 1;

    k_yield();

    // Même vérification après reprise
    if (k_current_task_state() != RUNNING)
        invariant_violated = 1;

    if (k_task_count_in_state(RUNNING) != 1)
        invariant_violated = 1;
}

/* Test 3 : invariant une seule task RUNNING */
static int test_invariant_running()
{
    invariant_violated = 0;
    k_init();
    k_task_create(fn_check_invariant, NULL);
    k_task_create(fn_check_invariant, NULL);
    k_run();
    k_shutdown();
    return invariant_violated ? -1 : 0;
}

static void fn_task4(void *args)
{
    test_ctx_t *ctx = args;
    *ctx->step = *ctx->step + 1;
    k_yield();
    *ctx->step = *ctx->step + 1;
    k_yield();
    *ctx->step = *ctx->step + 1;
}

static int test_une_tache_yield_multiple()
{
    int order[4];
    int step = 0;
    k_init();

    test_ctx_t ctx = {order, &step};

    k_task_create(fn_task4, &ctx);
    k_run();
    k_shutdown();
    if (step == 3)
    {
        return 0;
    }
    return -1;
}
static void fn_task5(void *args)
{
    (void)args;
}

static int test_toute_tache_finit()
{

    int order[4];
    int step = 0;
    k_init();

    test_ctx_t ctx = {order, &step};

    k_task_create(fn_task4, &ctx);
    k_task_create(fn_task5, NULL);

    k_run();
    k_shutdown();

    if (step == 3)
    {
        return 0;
    }
    return -1;
}

int main(void)
{
    assert(k_init() == K_OK);
    assert(k_init() == K_ERR_ALREADY_INIT);

    assert(k_shutdown() == K_OK);
    assert(k_shutdown() == K_ERR_NOT_INIT);

    assert(test_alternance() == 0);
    assert(test_etats() == 0);
    assert(test_invariant_running() == 0);
    assert(test_une_tache_yield_multiple() == 0);
    assert(test_toute_tache_finit() == 0);
    return 0;
}
