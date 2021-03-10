#include "foundation/memmgr.h"

void *memmgr_allocate_dirty(memmgr_ctx *ctx, size_t size)
{
    if (ctx) {
        ctx->n_allocs++;
    }

    void *ptr = malloc(size);
    if (!ptr) {
        die("allocation failed: %zu", size);
    }
    return ptr;
}

void *memmgr_allocate_clear(memmgr_ctx *ctx, size_t size)
{
    if (ctx) {
        ctx->n_allocs++;
    }

    void *ptr = calloc(1, size);
    if (!ptr) {
        die("allocation failed: %zu", size);
    }
    return ptr;
}

void memmgr_release(memmgr_ctx *ctx, void *ptr)
{
    if (ctx && ptr) {
        ctx->n_allocs--;
    }

    free(ptr);
}
