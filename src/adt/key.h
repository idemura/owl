#ifndef ADT_KEY_H
#define ADT_KEY_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Generic key. Consists of two parts: number (nk) and string (sk + skLen)
typedef struct {
  long nk;
  const char *sk;
  size_t skLen;
} Key;

static_assert(sizeof(Key) == 24, "key size");

inline static void Key_set(Key *key, long nk, const char *sk, size_t skLen) {
  key->nk = nk;
  key->sk = sk;
  key->skLen = skLen;
}

inline static Key Key_number(long n) {
  return (Key){.nk = n};
}

inline static Key Key_string(Key *key, const char *s) {
  return (Key){.sk = s, .skLen = strlen(s)};
}

inline static int Key_compare(const Key *a, const Key *b) {
  if (!(a->skLen | b->skLen) || a->nk != b->nk) {
    return (int) (a->nk - b->nk);
  }
  if (!b->skLen) {
    return 1;
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
