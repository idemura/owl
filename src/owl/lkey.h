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
      unsigned long skLen;
      unsigned long hk; // Hash value
    };
    long nk;
  };
} LKey;

static_assert(sizeof(LKey) == 24, "key_size");

inline static LKey LKey_number(long n) {
  return (LKey){.nk = n};
}

inline static LKey LKey_string(const char *s) {
  return (LKey){.sk = s, .skLen = strlen(s)};
}

inline static bool LKey_equals(const LKey *a, const LKey *b) {
  if (!a->sk) {
    return a->nk == b->nk;
  }
  if (a->skLen != b->skLen || a->hk != b->hk) {
    return 0;
  }
  return memcmp(a->sk, b->sk, a->skLen) == 0;
}

inline static int LKey_compare(const LKey *a, const LKey *b) {
  // All keys are either numbers or strings. Check if this is number first.
  if (!a->sk) {
    return (int) (a->nk - b->nk);
  }
  if (!b->skLen) {
    return a->skLen > 0;
  }
  if (!a->skLen) {
    return -1;
  }
  size_t cmpBytes = a->skLen < b->skLen ? a->skLen : b->skLen;
  int d = memcmp(a->sk, b->sk, cmpBytes);
  if (d != 0) {
    return d;
  }
  return (int) (a->skLen - b->skLen);
}

#endif
