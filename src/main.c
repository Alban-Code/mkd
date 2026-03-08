#include <stdio.h>
#include <string.h>

#include "kernel/kernel.h"
#include "kernel/log.h"
#include "kernel/task.h"

void fonction1(void *args)
{
    (void)args;
    LOG(LOG_INFO, "Je suis dans la fonction 1 avant le premier yield");
    k_yield();
    LOG(LOG_INFO, "Je suis dans la fonction 1 après le premier yield");
}

void fonction2(void *args)
{
    (void)args;
    LOG(LOG_INFO, "Je suis dans la fonction 2 avant le premier yield");
    k_yield();
    LOG(LOG_INFO, "Je suis dans la fonction 2 après le premier yield");
}

int main(int argc, char **argv)
{
    log_init(LOG_INFO);
    if (argc == 2 && strcmp(argv[1], "--selftest") == 0)
    {
        int selfest_return = k_selftest();
        if (selfest_return == 0)
        {
            LOG(LOG_INFO, "Selftest successful");
        }
        return selfest_return;
    }

    if (argc == 1)
    {
        k_init();
        k_task_create(fonction1, NULL);
        k_task_create(fonction2, NULL);
        k_run();
        k_shutdown();
        return 0;
    }

    fprintf(stderr, "Usage: %s [selftest]\n", argv[0]);
    return 0;
}