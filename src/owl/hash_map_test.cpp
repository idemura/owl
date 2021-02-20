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
    return k->nk;
}

static std::string to_string(skey_t key)
{
    return key.sk ? std::string{key.sk} : std::to_string(key.nk);
}

static const hash_map_entry *get_entry(const hash_map *h, size_t i)
{
    return HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * i);
}

TEST(hash_map, put_get)
{
    mm_test_ctx mm_ctx{};
    hash_map h =
            hash_map_new(skey_compare, skey_hash, get_memmgr_for_test(), &mm_ctx, sizeof(int), 4);
    EXPECT_EQ(0, hash_map_size(&h));

    *(int *) hash_map_put(&h, skey_number(1)) = 10;
    EXPECT_EQ(1, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));

    *(int *) hash_map_put(&h, skey_number(2)) = 20;
    EXPECT_EQ(2, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));

    *(int *) hash_map_put(&h, skey_number(6)) = 30;
    EXPECT_EQ(3, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(6)));

    *(int *) hash_map_put(&h, skey_number(5)) = 40;
    EXPECT_EQ(4, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(40, *(int *) hash_map_get(&h, skey_number(5)));

    *(int *) hash_map_put(&h, skey_number(13)) = 50;
    EXPECT_EQ(5, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(40, *(int *) hash_map_get(&h, skey_number(5)));
    EXPECT_EQ(50, *(int *) hash_map_get(&h, skey_number(13)));

    *(int *) hash_map_put(&h, skey_number(21)) = 60;
    EXPECT_EQ(6, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(40, *(int *) hash_map_get(&h, skey_number(5)));
    EXPECT_EQ(50, *(int *) hash_map_get(&h, skey_number(13)));
    EXPECT_EQ(60, *(int *) hash_map_get(&h, skey_number(21)));

    EXPECT_EQ(6, get_entry(&h, 0)->hash);
    EXPECT_EQ(1, get_entry(&h, 1)->hash);
    EXPECT_EQ(2, get_entry(&h, 2)->hash);
    EXPECT_EQ(0, get_entry(&h, 3)->hash);
    EXPECT_EQ(0, get_entry(&h, 4)->hash);
    EXPECT_EQ(5, get_entry(&h, 5)->hash);
    EXPECT_EQ(13, get_entry(&h, 6)->hash);
    EXPECT_EQ(21, get_entry(&h, 7)->hash);

    hash_map_destroy(&h);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(hash_map, put_del_1)
{
    mm_test_ctx mm_ctx{};
    hash_map h =
            hash_map_new(skey_compare, skey_hash, get_memmgr_for_test(), &mm_ctx, sizeof(int), 8);

    *(int *) hash_map_put(&h, skey_number(1)) = 10;
    *(int *) hash_map_put(&h, skey_number(2)) = 20;
    *(int *) hash_map_put(&h, skey_number(6)) = 30;
    *(int *) hash_map_put(&h, skey_number(5)) = 40;
    *(int *) hash_map_put(&h, skey_number(13)) = 50;
    *(int *) hash_map_put(&h, skey_number(21)) = 60;

    EXPECT_TRUE(hash_map_del(&h, skey_number(5)));

    EXPECT_EQ(5, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(2)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(50, *(int *) hash_map_get(&h, skey_number(13)));
    EXPECT_EQ(60, *(int *) hash_map_get(&h, skey_number(21)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(5)));

    hash_map_destroy(&h);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(hash_map, put_del_2)
{
    mm_test_ctx mm_ctx{};
    hash_map h =
            hash_map_new(skey_compare, skey_hash, get_memmgr_for_test(), &mm_ctx, sizeof(int), 8);

    *(int *) hash_map_put(&h, skey_number(6)) = 10;
    *(int *) hash_map_put(&h, skey_number(14)) = 20;
    *(int *) hash_map_put(&h, skey_number(22)) = 30;
    *(int *) hash_map_put(&h, skey_number(30)) = 40;

    EXPECT_TRUE(hash_map_del(&h, skey_number(6)));

    EXPECT_EQ(3, hash_map_size(&h));
    EXPECT_EQ(20, *(int *) hash_map_get(&h, skey_number(14)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(22)));
    EXPECT_EQ(40, *(int *) hash_map_get(&h, skey_number(30)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(6)));

    hash_map_destroy(&h);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(hash_map, put_del_3)
{
    mm_test_ctx mm_ctx{};
    hash_map h =
            hash_map_new(skey_compare, skey_hash, get_memmgr_for_test(), &mm_ctx, sizeof(int), 8);

    *(int *) hash_map_put(&h, skey_number(1)) = 10;
    *(int *) hash_map_put(&h, skey_number(9)) = 20;
    *(int *) hash_map_put(&h, skey_number(6)) = 30;

    EXPECT_TRUE(hash_map_del(&h, skey_number(9)));

    EXPECT_EQ(2, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(30, *(int *) hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(9)));

    EXPECT_TRUE(hash_map_del(&h, skey_number(6)));

    EXPECT_EQ(1, hash_map_size(&h));
    EXPECT_EQ(10, *(int *) hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(9)));

    EXPECT_TRUE(hash_map_del(&h, skey_number(1)));

    EXPECT_EQ(0, hash_map_size(&h));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(1)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(6)));
    EXPECT_EQ(NULL, hash_map_get(&h, skey_number(9)));

    hash_map_destroy(&h);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}
