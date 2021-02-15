#include "owl/hash_map.h"

hash_map hash_map_new(skey_hash_fn hash_key, memmgr *mm, void *mm_ctx, size_t value_size)
{
    hash_map h = {.hash_key = hash_key};
    return h;
}

void hash_map_destroy(hash_map *t)
{
    // Noop
}
