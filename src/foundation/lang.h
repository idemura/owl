#ifndef FOUNDATION_LANG_H
#define FOUNDATION_LANG_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

// Hungarian notation:
// I - i32
// J - u32 (next to I)
// L - i64
// M - u64 (next to L)
// U - size_t
// Z - strz
// A - array (pointer + size)
// P - pointer
// F - f32
// D - f64

#endif
