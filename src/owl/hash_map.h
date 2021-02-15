#ifndef OWL_HASH_MAP_H
#define OWL_HASH_MAP_H

#include "foundation/lang.h"
#include "owl/memmgr.h"
#include "owl/skey.h"

#ifdef __cplusplus
extern "C" {
#endif

// Hash map implementation based on Robin Hood hashing.

typedef struct {
    skey_hash_fn hash_key;
    size_t size;
} hash_map;

hash_map hash_map_new(skey_hash_fn hash_key, memmgr *mm, void *mm_ctx, size_t value_size);

hash_map hash_map_clone(const hash_map *t);

void hash_map_destroy(hash_map *t);

inline static size_t hash_map_size(const hash_map *t)
{
    return t->size;
}

void *hash_map_put(hash_map *t, skey_t key);
void *hash_map_get(hash_map *t, skey_t key);
bool hash_map_del(hash_map *t, skey_t key);

#ifdef __cplusplus
}
#endif

#endif
