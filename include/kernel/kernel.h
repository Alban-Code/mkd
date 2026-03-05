#ifndef KERNEL_H
#define KERNEL_H

typedef enum
{
    K_OK,
    K_ERR_ALREADY_INIT,
    K_ERR_NOT_INIT,
} k_status_t;

k_status_t k_init();

k_status_t k_shutdown();

int k_selftest();

#endif