#ifndef OWL_SKEY_H
#define OWL_SKEY_H

#include "foundation/lang.h"

#ifdef __cplusplus
extern "C" {
#endif

// Simple key for maps: natively supports long integers and strings.

typedef struct {
    long nk;
    const char *sk;
} skey_t;

typedef long (*skey_compare_fn)(const skey_t *a, const skey_t *b);
typedef uint64_t (*skey_hash_fn)(const skey_t *k);

inline static skey_t skey_number(long n)
{
    return (skey_t){.nk = n};
}

inline static skey_t skey_strn(const char *s, size_t s_len)
{
    return (skey_t){.nk = (long) s_len, .sk = s};
}

inline static skey_t skey_strz(const char *s)
{
    return skey_strn(s, strlen(s));
}

#ifdef __cplusplus
}
#endif

#endif
