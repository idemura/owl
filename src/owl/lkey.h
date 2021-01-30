#ifndef OWL_LKEY_H
#define OWL_LKEY_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Generic key. It is either number (long) or string with length and hash value.
typedef struct {
    const char *sk;
    union {
        struct {
            unsigned long sk_len;
            unsigned long hk; // Hash value
        };
        long nk;
    };
} lkey;

static_assert(sizeof(lkey) == 24, "key_size");

inline static lkey lkey_number(long n)
{
    return (lkey){.nk = n};
}

inline static lkey lkey_string(const char *s)
{
    return (lkey){.sk = s, .sk_len = strlen(s)};
}

inline static bool lkey_equals(const lkey *a, const lkey *b)
{
    if (!a->sk) {
        return a->nk == b->nk;
    }
    if (a->sk_len != b->sk_len || a->hk != b->hk) {
        return 0;
    }
    return memcmp(a->sk, b->sk, a->sk_len) == 0;
}

inline static int lkey_compare(const lkey *a, const lkey *b)
{
    // All keys are either numbers or strings. Check if this is number first.
    if (!a->sk) {
        return (int) (a->nk - b->nk);
    }
    if (!b->sk_len) {
        return a->sk_len > 0;
    }
    if (!a->sk_len) {
        return -1;
    }
    size_t cmp_bytes = a->sk_len < b->sk_len ? a->sk_len : b->sk_len;
    int d = memcmp(a->sk, b->sk, cmp_bytes);
    if (d != 0) {
        return d;
    }
    return (int) (a->sk_len - b->sk_len);
}

#endif
