#include "foundation/memmgr.h"

static void *stdmm_allocate_dirty(void *ctx, size_t size)
{
    return malloc(size);
}

static void *stdmm_allocate_clear(void *ctx, size_t size)
{
    return calloc(1, size);
}

static void stdmm_release(void *ctx, void *ptr)
{
    free(ptr);
}

const memmgr *get_memmgr(void)
{
    // clang-format off
    static const memmgr vtbl_mm = {
        .allocate_dirty = stdmm_allocate_dirty,
        .allocate_clear = stdmm_allocate_clear,
        .release = stdmm_release
    };
    // clang-format on

    return &vtbl_mm;
}

//
// Memory manager for testing
//

static void *test_allocate_dirty(void *p_ctx, size_t size)
{
    if (p_ctx) {
        mm_test_ctx *ctx = p_ctx;
        ctx->n_allocs++;
    }
    return stdmm_allocate_dirty(NULL, size);
}

static void *test_allocate_clear(void *p_ctx, size_t size)
{
    if (p_ctx) {
        mm_test_ctx *ctx = p_ctx;
        ctx->n_allocs++;
    }
    return stdmm_allocate_clear(NULL, size);
}

static void test_release(void *p_ctx, void *ptr)
{
    if (p_ctx && ptr) {
        mm_test_ctx *ctx = p_ctx;
        ctx->n_allocs--;
    }
    stdmm_release(NULL, ptr);
}

const memmgr *get_memmgr_for_test(void)
{
    // clang-format off
    static memmgr vtbl_mm = {
        .allocate_dirty = test_allocate_dirty,
        .allocate_clear = test_allocate_clear,
        .release = test_release
    };
    // clang-format off

    return &vtbl_mm;
}
