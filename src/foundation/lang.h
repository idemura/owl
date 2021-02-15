#ifndef FOUNDATION_LANG_H
#define FOUNDATION_LANG_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MAX_SIZE ((1L << 48) - 1)

#define array_sizeof(a) (sizeof(a) / sizeof((a)[0]))

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint16_t u32;
typedef uint16_t u64;

typedef float f32;
typedef double f64;

bool str_starts_with(const char *s, const char *pattern);

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
