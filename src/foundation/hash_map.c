#include "foundation/hash_map.h"

#include <stdio.h>

#define HASH_0_REPLACE 31

static bool is_power_of_2(size_t x)
{
    return (x & (x - 1)) == 0;
}

ATTR_NO_INLINE
static hash_map_entry *hash_map_alloc(const hash_map *h, size_t capacity)
{
    void *array = memmgr_allocate_dirty(h->mmc, h->entry_size * capacity);
    hash_map_entry *iter = array;
    for (size_t i = 0; i < capacity; i++) {
        iter->hash = 0;
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }
    return array;
}

inline static size_t hash_map_psl(uint64_t hash, size_t i, size_t capacity)
{
    return (i + capacity - (hash & (capacity - 1))) & (capacity - 1);
}

hash_map hash_map_new(
        skey_compare_fn compare_keys,
        skey_hash_fn hash_key,
        memmgr_ctx *mmc,
        size_t value_size,
        size_t capacity)
{
    value_size = pad_size_l(value_size);

    // clang-format off
    hash_map h = {
            .compare_keys = compare_keys,
            .hash_key = hash_key,
            .capacity = capacity,
            .entry_size = sizeof(hash_map_entry) + value_size,
            .mmc = mmc,
    };
    // clang-format on

    // Make swaps/moves around possible with stack memory and adequate performance.
    if (h.entry_size > HASH_MAP_MAX_ENTRY_SIZE) {
        die("hash_map: entry size (%zu): too large", h.entry_size);
    }

    if (capacity > 0) {
        if (!is_power_of_2(capacity)) {
            die("hash_map: capacity (%zu): must be power of two", capacity);
        }
        h.array = hash_map_alloc(&h, capacity);
        h.capacity = capacity;
    }
    return h;
}

void hash_map_destroy(hash_map *h)
{
    memmgr_release(h->mmc, h->array);
    *h = (hash_map){0};
}

void hash_map_print(const hash_map *h)
{
    if (h->size == 0) {
        printf("(empty)\n");
        return;
    }

    hash_map_entry *iter = h->array;
    for (size_t i = 0; i < h->capacity; i++) {
        if (iter->hash == 0) {
            printf("[%zu] empty\n", i);
        } else {
            printf("[%zu] psl=%zu hash=0x%08llx (%llu)\n",
                    i,
                    hash_map_psl(iter->hash, i, h->capacity),
                    (unsigned long long) iter->hash,
                    (unsigned long long) iter->hash);
        }
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }
}

static void hash_map_swap(void *a, void *b, size_t entry_size)
{
    unsigned long *lp_a = a;
    unsigned long *lp_b = b;
    for (size_t i = 0; i < entry_size; i += sizeof(long)) {
        unsigned long t = *lp_a;
        *lp_a++ = *lp_b;
        *lp_b++ = t;
    }
}

static void hash_map_reinsert(void *array, size_t capacity, size_t entry_size, hash_map_entry *e)
{
    size_t p = e->hash & (capacity - 1);
    size_t psl = 0;

    hash_map_entry *iter = HASH_MAP_ENTRY_OFFSET(array, entry_size * p);
    for (size_t i = p; i < capacity; i++) {
        if (iter->hash == 0) {
            memcpy(iter, e, entry_size);
            return;
        }

        size_t d = hash_map_psl(iter->hash, i, capacity);
        if (d < psl) {
            hash_map_swap(iter, e, entry_size);
            psl = d;
        }

        psl++;
        iter = HASH_MAP_ENTRY_OFFSET(iter, entry_size);
    }

    // We guaranteed to find a slot, because capacity is less than size.
    iter = array;
    for (size_t i = 0; true; i++) {
        if (iter->hash == 0) {
            memcpy(iter, e, entry_size);
            return;
        }

        size_t d = hash_map_psl(iter->hash, i, capacity);
        if (d < psl) {
            hash_map_swap(iter, e, entry_size);
            psl = d;
        }

        psl++;
        iter = HASH_MAP_ENTRY_OFFSET(iter, entry_size);
    }
}

ATTR_NO_INLINE
static void hash_map_double_capacity(hash_map *h)
{
    size_t double_cap = h->capacity * 2;
    if (double_cap < HASH_MAP_MIN_CAPACITY) {
        double_cap = HASH_MAP_MIN_CAPACITY;
    }

    hash_map_entry *array = hash_map_alloc(h, double_cap);
    if (!array) {
        die("hash_map: OOM (allocating %zu bytes)", h->entry_size * double_cap);
    }

    // Copy existing entries to the new array
    hash_map_entry *iter = h->array;
    for (size_t i = 0; i < h->capacity; i++) {
        if (iter->hash != 0) {
            hash_map_reinsert(array, double_cap, h->entry_size, iter);
        }
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }

    memmgr_release(h->mmc, h->array);
    h->array = array;
    h->capacity = double_cap;
}

inline static void hash_map_set_entry(
        hash_map_entry *e, uint64_t hash, skey_t key_value, size_t entry_size)
{
    *e = (hash_map_entry){.hash = hash};
    memcpy(e->value, key_value.ptr, entry_size - sizeof(hash_map_entry));
}

void hash_map_put(hash_map *h, skey_t key_value)
{
    if (10 * (h->size + 1) > 9 * h->capacity) {
        hash_map_double_capacity(h);
    }

    uint64_t hash = h->hash_key(key_value);
    if (hash == 0) {
        hash = HASH_0_REPLACE;
    }

    size_t p = hash & (h->capacity - 1); // Preferred location

    // Fast path
    hash_map_entry *iter = HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * p);
    if (iter->hash == 0) {
        hash_map_set_entry(iter, hash, key_value, h->entry_size);
        h->size++;
        return;
    }

    union {
        hash_map_entry e;
        char buf[HASH_MAP_MAX_ENTRY_SIZE];
    } u;

    hash_map_set_entry(&u.e, hash, key_value, h->entry_size);

    void *result = NULL;
    size_t psl = 0;

    for (size_t i = p; i < h->capacity; i++) {
        if (iter->hash == 0) {
            memcpy(iter, &u.e, h->entry_size);
            h->size++;
            return;
        }

        if (iter->hash == hash && h->compare_keys(SKEY_OF(iter->value), SKEY_OF(u.e.value)) == 0) {
            memcpy(iter, &u.e, h->entry_size);
            return;
        }

        size_t d = hash_map_psl(iter->hash, i, h->capacity);
        if (d < psl) {
            hash_map_swap(iter, &u.e, h->entry_size);
            psl = d;
            if (result == NULL) {
                result = iter->value;
            }
        }

        psl++;
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }

    // We guaranteed to find a slot, because capacity is less than size.
    iter = h->array;
    for (size_t i = 0; true; i++) {
        if (iter->hash == 0) {
            memcpy(iter, &u.e, h->entry_size);
            h->size++;
            return;
        }

        if (iter->hash == hash && h->compare_keys(SKEY_OF(iter->value), SKEY_OF(u.e.value)) == 0) {
            memcpy(iter, &u.e, h->entry_size);
            return;
        }

        size_t d = hash_map_psl(iter->hash, i, h->capacity);
        if (d < psl) {
            hash_map_swap(iter, &u.e, h->entry_size);
            psl = d;
            if (result == NULL) {
                result = iter->value;
            }
        }

        psl++;
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }
}

hash_map_entry *hash_map_find_entry(hash_map *h, skey_t key, size_t *entry_index)
{
    uint64_t hash = h->hash_key(key);
    if (hash == 0) {
        hash = HASH_0_REPLACE;
    }

    size_t p = hash & (h->capacity - 1); // Preferred location
    size_t psl = 0;

    hash_map_entry *iter = HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * p);
    for (size_t i = p; i < h->capacity; i++) {
        if (iter->hash == 0) {
            return NULL;
        }
        if (iter->hash == hash && h->compare_keys(SKEY_OF(iter->value), key) == 0) {
            *entry_index = i;
            return iter;
        }
        if (hash_map_psl(hash, i, h->capacity) < psl) {
            return NULL;
        }
        psl++;
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }

    // We guaranteed to find a slot, because capacity is less than size.
    iter = h->array;
    for (size_t i = 0; true; i++) {
        if (iter->hash == 0) {
            return NULL;
        }
        if (iter->hash == hash && h->compare_keys(SKEY_OF(iter->value), key) == 0) {
            *entry_index = i;
            return iter;
        }
        if (hash_map_psl(hash, i, h->capacity) < psl) {
            return NULL;
        }
        psl++;
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }

    // Just in case
    return NULL;
}

void *hash_map_get(hash_map *h, skey_t key)
{
    size_t i = 0;
    hash_map_entry *e = hash_map_find_entry(h, key, &i);
    return e == NULL ? NULL : e->value;
}

// Move range [first + 1, last) one position back
static void hash_map_move_entries(hash_map_entry *first, hash_map_entry *last, size_t entry_size)
{
    size_t n_bytes = HASH_MAP_ENTRY_DIFF(last, first) - entry_size;
    memmove(first, HASH_MAP_ENTRY_OFFSET(first, entry_size), n_bytes);
}

void hash_map_del(hash_map *h, skey_t key)
{
    size_t i = 0;
    hash_map_entry *found = hash_map_find_entry(h, key, &i);
    if (found == NULL) {
        return;
    }

    // Starting with the next to found, look for empty slot or one with PSL == 0.
    hash_map_entry *iter = HASH_MAP_ENTRY_OFFSET(found, h->entry_size);
    size_t j = i + 1;
    for (; j < h->capacity; j++) {
        if (iter->hash == 0 || hash_map_psl(iter->hash, j, h->capacity) == 0) {
            break;
        }
        iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
    }
    if (j - i > 1) {
        // Move entries i + 1, ..., j (exclusive) one position back
        hash_map_move_entries(found, iter, h->entry_size);
    }

    if (j == h->capacity) {
        // Wrap around. Move first entry to the last.
        iter = h->array;
        j = 0;
        for (; true; j++) {
            if (iter->hash == 0 || hash_map_psl(iter->hash, j, h->capacity) == 0) {
                break;
            }
            iter = HASH_MAP_ENTRY_OFFSET(iter, h->entry_size);
        }
        if (j > 0) {
            memcpy(HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * (h->capacity - 1)),
                    h->array,
                    h->entry_size);
        }
        hash_map_move_entries(h->array, iter, h->entry_size);
    }

    // (j - 1) modulo hash map capacity (which is power of 2)
    size_t j_1 = (h->capacity + j - 1) & (h->capacity - 1);
    memset(HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * j_1), 0, h->entry_size);

    h->size--;
}

void *hash_map_begin(hash_map *h, hash_map_iter *iter)
{
    iter->curr = 0;
    iter->h = h;

    if (h->size == 0) {
        return NULL;
    }

    hash_map_entry *e = h->array;
    while (e->hash == 0) {
        e = HASH_MAP_ENTRY_OFFSET(e, h->entry_size);
        iter->curr++;
    }

    return e->value;
}

void *hash_map_iter_next(hash_map_iter *iter)
{
    hash_map *h = iter->h;

    iter->curr++;
    hash_map_entry *e = HASH_MAP_ENTRY_OFFSET(h->array, h->entry_size * iter->curr);
    while (iter->curr < h->capacity && e->hash == 0) {
        e = HASH_MAP_ENTRY_OFFSET(e, h->entry_size);
        iter->curr++;
    }

    return iter->curr < h->capacity ? e->value : NULL;
}
