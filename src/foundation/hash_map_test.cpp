#include "foundation/hash_map.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <map>
#include <string>

struct key_value {
    int k = 0;
    int v = 0;

    key_value(int k_, int v_): k{k_}, v{v_} {}
};

static long skey_compare(skey_t a, skey_t b)
{
    return *((const int *) a.ptr) - *((const int *) b.ptr);
}

static uint64_t skey_hash(skey_t a)
{
    return *((const int *) a.ptr);
}

static std::string to_string(skey_t key)
{
    return std::to_string(*(const int *) key.ptr);
}

static const hash_map_entry *get_entry(const hash_map *h, size_t i)
{
    return HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * i);
}

TEST(hash_map, put_get)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 4);
    EXPECT_EQ(0, hash_map_size(&h));

    hash_map_put_v(&h, key_value(1, 10));
    EXPECT_EQ(1, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);

    hash_map_put_v(&h, key_value(2, 20));
    EXPECT_EQ(2, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 2))->v);

    hash_map_put_v(&h, key_value(6, 30));
    EXPECT_EQ(3, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 2))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 6))->v);

    hash_map_put_v(&h, key_value(5, 40));
    EXPECT_EQ(4, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 2))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 6))->v);
    EXPECT_EQ(40, ((key_value *) hash_map_get_v(&h, 5))->v);

    hash_map_put_v(&h, key_value(13, 50));
    EXPECT_EQ(5, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 2))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 6))->v);
    EXPECT_EQ(40, ((key_value *) hash_map_get_v(&h, 5))->v);
    EXPECT_EQ(50, ((key_value *) hash_map_get_v(&h, 13))->v);

    hash_map_put_v(&h, key_value(21, 60));
    EXPECT_EQ(6, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 2))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 6))->v);
    EXPECT_EQ(40, ((key_value *) hash_map_get_v(&h, 5))->v);
    EXPECT_EQ(50, ((key_value *) hash_map_get_v(&h, 13))->v);
    EXPECT_EQ(60, ((key_value *) hash_map_get_v(&h, 21))->v);

    EXPECT_EQ(6, get_entry(&h, 0)->hash);
    EXPECT_EQ(1, get_entry(&h, 1)->hash);
    EXPECT_EQ(2, get_entry(&h, 2)->hash);
    EXPECT_EQ(0, get_entry(&h, 3)->hash);
    EXPECT_EQ(0, get_entry(&h, 4)->hash);
    EXPECT_EQ(5, get_entry(&h, 5)->hash);
    EXPECT_EQ(13, get_entry(&h, 6)->hash);
    EXPECT_EQ(21, get_entry(&h, 7)->hash);

    hash_map_destroy(&h);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(hash_map, put_del_1)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 8);

    hash_map_put_v(&h, key_value(1, 10));
    hash_map_put_v(&h, key_value(2, 20));
    hash_map_put_v(&h, key_value(6, 30));
    hash_map_put_v(&h, key_value(5, 40));
    hash_map_put_v(&h, key_value(13, 50));
    hash_map_put_v(&h, key_value(21, 60));

    hash_map_del_v(&h, 5);

    EXPECT_EQ(5, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 2))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 6))->v);
    EXPECT_EQ(50, ((key_value *) hash_map_get_v(&h, 13))->v);
    EXPECT_EQ(60, ((key_value *) hash_map_get_v(&h, 21))->v);
    EXPECT_EQ(NULL, hash_map_get_v(&h, 5));

    hash_map_destroy(&h);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(hash_map, put_del_2)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 8);

    hash_map_put_v(&h, key_value(6, 10));
    hash_map_put_v(&h, key_value(14, 20));
    hash_map_put_v(&h, key_value(22, 30));
    hash_map_put_v(&h, key_value(30, 40));

    hash_map_del_v(&h, 6);

    EXPECT_EQ(3, hash_map_size(&h));
    EXPECT_EQ(20, ((key_value *) hash_map_get_v(&h, 14))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 22))->v);
    EXPECT_EQ(40, ((key_value *) hash_map_get_v(&h, 30))->v);
    EXPECT_EQ(NULL, hash_map_get_v(&h, 6));

    hash_map_destroy(&h);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(hash_map, put_del_3)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 8);

    hash_map_put_v(&h, key_value(1, 10));
    hash_map_put_v(&h, key_value(9, 20));
    hash_map_put_v(&h, key_value(6, 30));

    hash_map_del_v(&h, 9);

    EXPECT_EQ(2, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(30, ((key_value *) hash_map_get_v(&h, 6))->v);
    EXPECT_EQ(NULL, hash_map_get_v(&h, 9));

    hash_map_del_v(&h, 6);

    EXPECT_EQ(1, hash_map_size(&h));
    EXPECT_EQ(10, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(NULL, hash_map_get_v(&h, 6));
    EXPECT_EQ(NULL, hash_map_get_v(&h, 9));

    hash_map_del_v(&h, 1);

    EXPECT_EQ(0, hash_map_size(&h));
    EXPECT_EQ(NULL, hash_map_get_v(&h, 1));
    EXPECT_EQ(NULL, hash_map_get_v(&h, 6));
    EXPECT_EQ(NULL, hash_map_get_v(&h, 9));

    hash_map_destroy(&h);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(hash_map, replace)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 8);

    hash_map_put_v(&h, key_value(1, 10));
    hash_map_put_v(&h, key_value(9, 20));
    hash_map_put_v(&h, key_value(6, 30));

    hash_map_put_v(&h, key_value(1, 11));
    hash_map_put_v(&h, key_value(9, 21));
    hash_map_put_v(&h, key_value(6, 31));

    EXPECT_EQ(3, hash_map_size(&h));
    EXPECT_EQ(11, ((key_value *) hash_map_get_v(&h, 1))->v);
    EXPECT_EQ(21, ((key_value *) hash_map_get_v(&h, 9))->v);
    EXPECT_EQ(31, ((key_value *) hash_map_get_v(&h, 6))->v);

    hash_map_destroy(&h);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(hash_map, iterator_empty)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 8);

    hash_map_iter iter;
    EXPECT_EQ(nullptr, hash_map_begin(&h, &iter));
}

TEST(hash_map, iterator)
{
    memmgr_ctx mmc{};
    hash_map h = hash_map_new(skey_compare, skey_hash, &mmc, sizeof(key_value), 8);

    hash_map_put_v(&h, key_value(1, 10));
    hash_map_put_v(&h, key_value(9, 20));
    hash_map_put_v(&h, key_value(6, 30));

    long keys[3] = {};
    size_t i = 0;

    hash_map_iter iter;
    for (void *v = hash_map_begin(&h, &iter); v != nullptr; v = hash_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(3, i);
    EXPECT_EQ(1, keys[0]);
    EXPECT_EQ(9, keys[1]);
    EXPECT_EQ(6, keys[2]);
}
