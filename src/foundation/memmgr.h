#ifndef FOUNDATION_MEMMGR_H
#define FOUNDATION_MEMMGR_H

#include "foundation/lang.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Memory manager/allocator.
 */

typedef struct {
    long n_allocs;
} memmgr_ctx;

inline static size_t pad_size_l(size_t size)
{
    return (size + sizeof(long) - 1) & ~(sizeof(long) - 1);
}

// Allocate dirty memory
void *memmgr_allocate_dirty(memmgr_ctx *ctx, size_t size);

// Allocate clear memory (init with 0)
void *memmgr_allocate_clear(memmgr_ctx *ctx, size_t size);

// Releases allocated memory (@ptr can be NULL).
void memmgr_release(memmgr_ctx *ctx, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
