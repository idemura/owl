#ifndef FOUNDATION_SKEY_H
#define FOUNDATION_SKEY_H

#include "foundation/lang.h"

#ifdef __cplusplus
extern "C" {
#endif

// Wraps pointer to a simple key (and optionally a value). We want a type distinct from just void*
// to see keys in the code clearly.
typedef struct {
    const void *ptr;
} skey_t;

typedef long (*skey_compare_fn)(skey_t a, skey_t b);
typedef uint64_t (*skey_hash_fn)(skey_t k);

#define SKEY_OF(p) ((skey_t){.ptr = (p)})

#ifdef __cplusplus
}
#endif

#endif
