#ifndef OWL_HASH_MAP_H
#define OWL_HASH_MAP_H

#include "foundation/lang.h"
#include "owl/memmgr.h"
#include "owl/skey.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_MAP_MAX_ENTRY_SIZE 256
#define HASH_MAP_MIN_CAPACITY 4
#define HASH_MAP_ENTRY_OFFSET(a, ofs) ((hash_map_entry *) ((char *) (a) + (ofs)))

// Hash map implementation based on Robin Hood hashing.
typedef struct {
    uint64_t hash;

    skey_t key;

    alignas(long) char value[];
} hash_map_entry;

typedef struct {
    skey_compare_fn compare_keys;
    skey_hash_fn hash_key;

    void *array;

    size_t size;
    size_t capacity;

    size_t entry_size;

    const memmgr *mm;
    void *mm_ctx;
} hash_map;

hash_map hash_map_new(
        skey_compare_fn compare_keys,
        skey_hash_fn hash_key,
        const memmgr *mm,
        void *mm_ctx,
        size_t value_size,
        size_t capacity);

hash_map hash_map_clone(const hash_map *h);

void hash_map_destroy(hash_map *h);

inline static size_t hash_map_size(const hash_map *h)
{
    return h->size;
}

void *hash_map_put(hash_map *h, skey_t key);
void *hash_map_get(hash_map *h, skey_t key);
bool hash_map_del(hash_map *h, skey_t key);
void hash_map_print(const hash_map *h);

#ifdef __cplusplus
}
#endif

#endif
