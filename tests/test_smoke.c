#include <assert.h>
#include "kernel/kernel.h"

int main(void)
{
    assert(k_init() == K_OK);
    assert(k_init() == K_ERR_ALREADY_INIT);

    assert(k_shutdown() == K_OK);
    assert(k_shutdown() == K_ERR_NOT_INIT);

    return 0;
}