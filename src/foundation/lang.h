#ifndef FOUNDATION_LANG_H
#define FOUNDATION_LANG_H

#include <assert.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

#define OWL_MAX_SIZE ((1L << 48) - 1)

#define array_sizeof(a) (sizeof(a) / sizeof((a)[0]))

#define ATTR_NO_INLINE __attribute__((noinline))

bool str_starts_with(const char *s, const char *pattern);
noreturn void die(const char *format, ...);

// Overload type notation:
// I - i32
// UI - u32 (next to I)
// L - i64
// UL - u64 (next to L)
// N - size_t
// SZ - strz
// A - array (pointer + size)
// P - pointer
// F32 - f32
// F64 - f64

#endif
