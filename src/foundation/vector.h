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
    memmgr_ctx *mmc;
    void *array;
} vector;

#define def_vector(T) \
    struct { \
        size_t size; \
        size_t capacity; \
        memmgr_ctx *mmc; \
        union { \
            T *array; \
            /* for C++ */ void *vp; \
        }; \
    }

#define vector_init(v) vector_init_with_ctx(v, NULL)

#define vector_init_with_ctx(v, ctx) \
    ({ \
        __typeof__(v) _v = v; \
        _v->size = 0; \
        _v->capacity = 0; \
        _v->mmc = (ctx); \
        _v->array = NULL; \
    })

#define vector_init_capacity(v, cap) vector_init_capacity_with_ctx(v, cap, NULL)

#define vector_init_capacity_with_ctx(v, cap, ctx) \
    ({ \
        __typeof__(v) _v = v; \
        _v->size = 0; \
        _v->capacity = (cap); \
        _v->mmc = (ctx); \
        _v->vp = memmgr_allocate_dirty(_v->mmc, sizeof(*_v->array) * _v->capacity); \
    })

#define vector_release(v) \
    ({ \
        __typeof__(v) _v = v; \
        memmgr_release(_v->mmc, _v->array); \
    })

// Add value to the end of the vector @v. @v is a pointer to the vector.
#define vector_add(v, val) \
    ({ \
        __typeof__(v) _v = v; \
        if (_v->size == _v->capacity) { \
            vector_increase_capacity((vector *) _v, sizeof(*_v->array), _v->size + 1); \
        } \
        _v->array[_v->size] = val; \
        _v->size++; \
        /* return */ _v->size; \
    })

// Add a value to the end n times (replicate).
#define vector_add_rep(v, val, n) \
    ({ \
        __typeof__(v) _v = v; \
        size_t _n = n; \
        vector_increase_capacity((vector *) _v, sizeof(*_v->array), _v->size + _n); \
        __typeof__(sizeof(*_v->array)) _a = val; \
        for (size_t _i = 0; _i < _n; ++_i) { \
            _v->array[_v->size + _i] = _a; \
        } \
        _v->size += _n; \
        /* return */ _v->size; \
    })

// Add a list of value to the end
#define vector_add_arr(v, ptr, n) \
    ({ \
        __typeof__(v) _v = v; \
        size_t _n = n; \
        vector_increase_capacity((vector *) _v, sizeof(*_v->array), _v->size + _n); \
        memcpy(_v->array + _v->size, (ptr), sizeof(*_v->array) * _n); \
        _v->size += _n; \
        /* return */ _v->size; \
    })

#define vector_pop(v) \
    ({ \
        __typeof__(v) _v = v; \
        _v->size--; \
        _v->array[_v->size]; \
    })

#define vector_remove(v, from, n) \
    ({ \
        __typeof__(v) _v = v; \
        vector_remove_impl((vector *) _v, sizeof(*_v->array), from, n); \
    })

#ifdef NDEBUG
#define vector_ptr(v, i) ((v)->array + (i))
#else
#define vector_ptr(v, i) \
    ({ \
        __typeof__(v) _v = v; \
        size_t _i = i; \
        vector_check_index((vector *) _v, _i); \
        _v->array + _i; \
    })
#endif

#define vector_get(v, i) *vector_ptr(v, i)

#define vector_size(v) (v)->size

#define vector_foreach(e, v) \
    for (__typeof__((v)->array) e = (v)->array, _last = (v)->array + (v)->size; e != _last; ++e)

void vector_check_index(const vector *v, size_t i);
void vector_increase_capacity(vector *v, size_t entry_size, size_t new_size);
void vector_remove_impl(vector *v, size_t entry_size, size_t first, size_t n);

#ifdef __cplusplus
}
#endif

#endif
