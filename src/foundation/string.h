#ifndef FOUNDATION_STRING_H
#define FOUNDATION_STRING_H

#include "foundation/lang.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dynamically allocated string
 */

typedef struct {
    size_t len;
    char *str;
} string;

extern char string_empty_buf[];

inline static string string_empty()
{
    return (string){.len = 0, .str = string_empty_buf};
}

inline static string string_of(char *str)
{
    return (string){.len = strlen(str), .str = str};
}

// Construct a string by making a copy.
string string_copy_of(const char *str);

// Release string buffer.
void string_release(string s);

#ifdef __cplusplus
}
#endif

#endif
