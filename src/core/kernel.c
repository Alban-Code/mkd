#include "kernel/kernel.h"
#include "internal/kernel_internal.h"
#include "kernel/log.h"

#include <stdlib.h>

kernel_t g_kernel;

k_status_t k_init()
{
    if (!g_kernel.initialized)
    {
        g_kernel.initialized = true;
        LOG(LOG_INFO, "kernel initialized");
        return K_OK;
    }
    else
        return K_ERR_ALREADY_INIT;
}

k_status_t k_shutdown()
{
    if (!g_kernel.initialized)
        return K_ERR_NOT_INIT;
    else
    {
        g_kernel.initialized = false;
        g_kernel.head = NULL;
        g_kernel.tail = NULL;
        g_kernel.current_task = NULL;
        g_kernel.to_free = NULL;
        LOG(LOG_INFO, "kernel shutdown");
        return K_OK;
    }
}

int k_selftest()
{
    k_status_t init = k_init();
    if (init != K_OK)
    {
        LOG(LOG_ERROR, "Kernel initialization unsucessful");
        return -1;
    }

    k_status_t second_init = k_init();
    if (second_init != K_ERR_ALREADY_INIT)
    {
        LOG(LOG_ERROR, "Second kernel initialization should not work");
        return -1;
    }

    k_status_t shutdown = k_shutdown();
    if (shutdown != K_OK)
    {
        LOG(LOG_ERROR, "Kernel shutdown unsucessful");
        return -1;
    }

    k_status_t second_shutdown = k_shutdown();
    if (second_shutdown != K_ERR_NOT_INIT)
    {
        LOG(LOG_ERROR, "Second kernel shutdown should not work");
        return -1;
    }

    return 0;
}