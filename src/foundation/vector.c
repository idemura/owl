#include "foundation/vector.h"

#define MIN_CAPACITY 4
#define DOUBLE_CAPACITY_MAX (1ul << 15) /* 32k => 64k entries is the last after double */

void vector_realloc(vector *v, size_t entry_size, size_t new_size)
{
    const memmgr *mm = get_memmgr();

    size_t cap = v->capacity;
    if (cap < MIN_CAPACITY) {
        cap = MIN_CAPACITY;
    }

    while (cap < new_size && cap <= DOUBLE_CAPACITY_MAX) {
        cap += cap;
    }

    while (cap < new_size && cap < OWL_MAX_SIZE) {
        // +50%
        cap += cap >> 1;
    }

    if (cap > OWL_MAX_SIZE) {
        die("vector: capacity over OWL_MAX_SIZE (%zu), new_size=", OWL_MAX_SIZE, new_size);
    }

    size_t byte_size = entry_size * cap;
    void *p = mm->allocate_dirty(NULL, byte_size);

    if (v->capacity > 0) {
        // Copy old data to the new memory and release current array
        memcpy(p, v->array, entry_size * v->size);
        mm->release(NULL, v->array);
    }

    v->capacity = cap;
    v->array = p;
}
