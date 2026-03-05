#include <stdio.h>
#include <string.h>

#include "kernel/kernel.h"
#include "kernel/log.h"

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
        k_shutdown();
        return 0;
    }

    fprintf(stderr, "Usage: %s [selftest]\n", argv[0]);
    return 0;
}