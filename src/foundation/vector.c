#include "foundation/vector.h"

#define MIN_CAPACITY 4

void *vector_realloc_(void *array, size_t entry_size, size_t *capacity)
{
    const memmgr *mm = get_memmgr();

    size_t cap = *capacity;
    if (cap < MIN_CAPACITY) {
        cap = MIN_CAPACITY;
    } else if (cap <= (1ul << 15)) {
        // +100%
        cap *= 2;
    } else {
        // +50%
        cap += cap >> 1;
    }

    if (cap > OWL_MAX_SIZE) {
        die("vector: capacity %zu over OWL_MAX_SIZE %zu", capacity, OWL_MAX_SIZE);
    }

    size_t byte_size = entry_size * cap;
    void *p = mm->allocate_dirty(NULL, byte_size);

    size_t curr_cap = *capacity;
    if (curr_cap > 0) {
        // Copy old data to the new memory
        memcpy(p, array, entry_size * curr_cap);

        mm->release(NULL, array);
    }

    *capacity = cap;
    return p;
}
