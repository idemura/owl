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
        union { \
            T *array; \
            /* for C++ */ void *vp; \
        }; \
    }

/**
 * Add value to the end of the vector @v. @v is a pointer to the vector.
 */
#define vector_add(v, val) \
    ({ \
        __typeof__(v) _v = v; \
        if (_v->size == _v->capacity) { \
            vector_realloc((vector *) _v, sizeof(*_v->array), _v->size + 1); \
        } \
        _v->array[_v->size] = val; \
        _v->size++; \
        /* return */ _v->size; \
    })

/**
 * Add value to the end of the vector @v n times.
 */
#define vector_add_n(v, val, n) \
    ({ \
        __typeof__(v) _v = v; \
        size_t _n = n; \
        __typeof__(sizeof(*_v->array)) _a = val; \
        vector_realloc((vector *) _v, sizeof(*_v->array), _v->size + _n); \
        for (size_t _i = 0; _i < _n; ++_i) { \
            _v->array[_v->size + _i] = _a; \
        } \
        _v->size += _n; \
        /* return */ _v->size; \
    })

#define vector_pop(v) \
    ({ \
        __typeof__(v) _v = v; \
        _v->size--; \
        _v->array[_v->size]; \
    })

#ifdef NDEBUG
#define vector_ptr_at(v, i) ((v)->array + (i))
#else
#define vector_ptr_at(v, i) \
    ({ \
        __typeof__(v) _v = v; \
        size_t _i = i; \
        if (_i >= _v->size) { \
            die("index %zu out of bounds: %zu", _i, _v->size); \
        } \
        _v->array + _i; \
    })
#endif

#define vector_at(v, i) *vector_ptr_at(v, i)

#define vector_init(v) \
    ({ \
        __typeof__(v) _v = v; \
        _v->size = 0; \
        _v->capacity = 0; \
        _v->array = NULL; \
    })

#define vector_init_capacity(v, cap) \
    ({ \
        __typeof__(v) _v = v; \
        _v->size = 0; \
        _v->capacity = cap; \
        _v->vp = get_memmgr()->allocate_dirty(NULL, sizeof(*_v->array) * cap); \
    })

#define vector_size(v) (v)->size

#define vector_foreach(e, v) \
    for (__typeof__((v)->array) e = (v)->array, _last = (v)->array + (v)->size; e != _last; ++e)

// Private function
void vector_realloc(vector *v, size_t entry_size, size_t new_size);

#ifdef __cplusplus
}
#endif

#endif
