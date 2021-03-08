#ifndef FOUNDATION_VECTOR_H
#define FOUNDATION_VECTOR_H

#include "foundation/lang.h"
#include "foundation/memmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dynamically sized vector implementation.
 */

typedef struct {
    size_t size;
    size_t capacity;

    void *array;
} vector;

#define def_vector_of(T) \
    struct vector_##T { \
        size_t size; \
        size_t capacity; \
        T *array; \
    }

/**
 * Add new value to the end of vector @v. @v is a pointer to the vector.
 */
#define vector_add(v, val) \
    ({ \
        __typeof__(v) vptr = v; \
        if (vptr->size == vptr->capacity) { \
            ((vector *) vptr)->array = \
                    vector_realloc_(vptr->array, sizeof(*vptr->array), &vptr->capacity); \
        } \
        vptr->array[vptr->size] = val; \
        vptr->size++; \
    })

#define vector_pop(v) \
    ({ \
        __typeof__(v) vptr = v; \
        vptr->size--; \
        vptr->array[vptr->size]; \
    })

#ifdef NDEBUG
#define vector_ptr_at(v, i) ((v)->array + (i))
#else
#define vector_ptr_at(v, i) \
    ({ \
        __typeof__(v) vptr = v; \
        size_t _i = i; \
        if (_i >= vptr->size) { \
            die("index %zu out of bounds: %zu", _i, vptr->size); \
        } \
        vptr->array + _i; \
    })
#endif

#define vector_at(v, i) *vector_ptr_at(v, i)

#define vector_init(v) \
    ({ \
        __typeof__(v) vptr = v; \
        vptr->size = 0; \
        vptr->capacity = 0; \
        vptr->array = NULL; \
    })

#define vector_init_capacity(v, cap) \
    ({ \
        __typeof__(v) vptr = v; \
        vptr->size = 0; \
        vptr->capacity = cap; \
        vptr->array = get_memgr()->allocate_dirty(sizeof(*vptr->array) * cap); \
    })

#define vector_size(v) (v)->size

#define vector_foreach(e, v) \
    for (__typeof__((v)->array) e = (v)->array, _last = (v)->array + (v)->size; e != _last; ++e)

// Private function
void *vector_realloc_(void *array, size_t entry_size, size_t *capacity);

#ifdef __cplusplus
}
#endif

#endif
