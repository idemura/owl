#include "owl/hash_map.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <map>
#include <string>

static long skey_compare(const skey_t *a, const skey_t *b)
{
    return a->nk - b->nk;
}

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
    mm_test_ctx mm_ctx{};
    hash_map h =
            hash_map_new(skey_compare, skey_hash, get_memmgr_for_test(), &mm_ctx, sizeof(int), 0);
    EXPECT_EQ(0, hash_map_size(&h));

    *(int *) hash_map_put(&h, skey_number(1)) = 10;
    EXPECT_EQ(1, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));

    *(int *) hash_map_put(&h, skey_number(2)) = 20;
    EXPECT_EQ(2, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));

    hash_map_print(&h);

    hash_map_destroy(&h);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}
