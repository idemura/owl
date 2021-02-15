#include "owl/memmgr.h"

static void *stdmm_allocatez(void *ctx, size_t size)
{
    return calloc(1, size);
}

static void stdmm_release(void *ctx, void *ptr)
{
    return free(ptr);
}

static memmgr vtbl_mm = {.allocatez = stdmm_allocatez, .release = stdmm_release};

memmgr *get_std_memmgr(void)
{
    return &vtbl_mm;
}
