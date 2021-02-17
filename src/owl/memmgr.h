#ifndef OWL_MEMMGR_H
#define OWL_MEMMGR_H

#include "foundation/lang.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory manager/allocator.

typedef struct {
    // Allocate dirty memory
    void *(*allocate_dirty)(void *ctx, size_t size);

    // Allocate clear memory (init with 0)
    void *(*allocate_clear)(void *ctx, size_t size);

    // Releases allocated memory (@ptr can be NULL).
    void (*release)(void *ctx, void *ptr);
} memmgr;

typedef struct {
    long n_allocs;
} mm_test_ctx;

const memmgr *get_memmgr(void);
const memmgr *get_memmgr_for_test(void);

inline static size_t pad_size_l(size_t size)
{
    return (size + sizeof(long) - 1) & ~(sizeof(long) - 1);
}

#ifdef __cplusplus
}
#endif

#endif
