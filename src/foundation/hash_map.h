#ifndef FOUNDATION_HASH_MAP_H
#define FOUNDATION_HASH_MAP_H

#include "foundation/lang.h"
#include "foundation/memmgr.h"
#include "foundation/skey.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_MAP_MAX_ENTRY_SIZE 256
#define HASH_MAP_MIN_CAPACITY 4
#define HASH_MAP_ENTRY_OFFSET(a, ofs) ((hash_map_entry *) ((char *) (a) + (ofs)))
#define HASH_MAP_ENTRY_DIFF(a, b) ((char *) (a) - (char *) (b))

/**
 * Hash map implementation based on Robin Hood hashing.
 */

typedef struct {
    uint64_t hash;

    alignas(long) unsigned char value[];
} hash_map_entry;

typedef struct {
    skey_compare_fn compare_keys;
    skey_hash_fn hash_key;

    void *array;

    size_t size;
    size_t capacity;

    size_t entry_size;

    memmgr_ctx *mmc;
} hash_map;

hash_map hash_map_new(
        skey_compare_fn compare_keys,
        skey_hash_fn hash_key,
        memmgr_ctx *mmc,
        size_t value_size,
        size_t capacity);

inline static hash_map hash_map_new_default(
        skey_compare_fn compare_keys, skey_hash_fn hash_key, size_t value_size)
{
    return hash_map_new(compare_keys, hash_key, NULL, value_size, 0);
}

hash_map hash_map_clone(const hash_map *h);

void hash_map_destroy(hash_map *h);

inline static size_t hash_map_size(const hash_map *h)
{
    return h->size;
}

void hash_map_put(hash_map *h, skey_t key_value);

#define hash_map_put_v(h, key_value) \
    ({ \
        __typeof__(key_value) lvalue = (key_value); \
        hash_map_put((h), SKEY_OF(&lvalue)); \
    })

void *hash_map_get(hash_map *h, skey_t key);

#define hash_map_get_v(h, key) \
    ({ \
        __typeof__(key) lvalue = (key); \
        hash_map_get((h), SKEY_OF(&lvalue)); \
    })

void hash_map_del(hash_map *h, skey_t key);

#define hash_map_del_v(h, key) \
    ({ \
        __typeof__(key) lvalue = (key); \
        hash_map_del((h), SKEY_OF(&lvalue)); \
    })

void hash_map_print(const hash_map *h);

typedef struct {
    size_t curr;
    hash_map *h;
} hash_map_iter;

// Init iterator.
void *hash_map_begin(hash_map *h, hash_map_iter *iter);

// Get next, or null if iteration complete.
void *hash_map_iter_next(hash_map_iter *iter);

#ifdef __cplusplus
}
#endif

#endif
