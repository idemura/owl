#include "foundation/vector.h"

#define MIN_CAPACITY 4
#define DOUBLE_CAPACITY_MAX (1ul << 15) /* 32k => 64k entries is the last after double */

void vector_check_index(const vector *v, size_t i)
{
    if (i >= v->size) {
        die("vector index %zu out of bounds: %zu", i, v->size);
    }
}

void vector_increase_capacity(vector *v, size_t entry_size, size_t new_size)
{
    if (new_size > OWL_MAX_SIZE) {
        die("vector: capacity over OWL_MAX_SIZE (%zu), new_size=%zu", OWL_MAX_SIZE, new_size);
    }

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
        cap = OWL_MAX_SIZE;
    }

    size_t byte_size = entry_size * cap;
    void *p = memmgr_allocate_dirty(v->mmc, byte_size);

    if (v->capacity > 0) {
        // Copy old data to the new memory and release current array
        memcpy(p, v->array, entry_size * v->size);
        memmgr_release(v->mmc, v->array);
    }

    v->capacity = cap;
    v->array = p;
}

void vector_remove_impl(vector *v, size_t entry_size, size_t first, size_t n)
{
    if (n == 0) {
        return;
    }

    size_t last = first + n;

#ifndef NDEBUG
    vector_check_index(v, first);
    vector_check_index(v, last - 1);
#endif

    char *a = (char *) v->array;
    memmove(a + entry_size * first, a + entry_size * last, entry_size * (v->size - last));
    v->size -= n;
}
