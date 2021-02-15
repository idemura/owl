#include "owl/hash_map.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <map>
#include <string>

struct test_memmgr_ctx {
    long n_nodes = 0;
};

static void *test_allocatez(void *ctx, size_t size)
{
    if (ctx) {
        auto *test_ctx = (test_memmgr_ctx *) ctx;
        test_ctx->n_nodes++;
    }
    return calloc(1, size);
}

static void test_release(void *ctx, void *ptr)
{
    if (ctx) {
        auto *test_ctx = (test_memmgr_ctx *) ctx;
        test_ctx->n_nodes--;
    }
    return free(ptr);
}

static memmgr vtbl_mm{.allocatez = test_allocatez, .release = test_release};

static uint64_t skey_hash(const skey_t *k)
{
    return k->nk * 31;
}

static std::string to_string(skey_t key)
{
    return key.sk ? std::string{key.sk} : std::to_string(key.nk);
}

TEST(hash_map, put_get)
{
    test_memmgr_ctx mm_ctx;
    hash_map h = hash_map_new(skey_hash, &vtbl_mm, &mm_ctx, sizeof(int));
    EXPECT_EQ(0, hash_map_size(&h));
    hash_map_destroy(&h);
}
