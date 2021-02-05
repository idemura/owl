#ifndef OWL_LKEY_H
#define OWL_LKEY_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    long nk;
} lkey;

inline static lkey lkey_number(long n)
{
    return (lkey){.nk = n};
}

static long lkey_compare(const lkey *a, const lkey *b)
{
    return a->nk - b->nk;
}

#ifdef __cplusplus
}
#endif

#endif
