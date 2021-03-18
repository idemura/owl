#ifndef FOUNDATION_STRING_H
#define FOUNDATION_STRING_H

#include "foundation/memmgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dynamically allocated string
 */

// String length must fit into int type range, because of %.*s format. We have limit of 16MB on
// string:
#define OWL_STRING_MAX ((size_t) 1u << 24)

typedef struct {
    size_t len;
    char *str;
} string;

extern char string_empty_buf[];

inline static string string_empty()
{
    return (string){.len = 0, .str = string_empty_buf};
}

inline static string string_of_len(char *str, size_t len)
{
    assert(len < OWL_STRING_MAX);
    return (string){.len = len, .str = str};
}

inline static string string_of(char *str)
{
    return string_of_len(str, strlen(str));
}

// Construct a string by making a copy.
string string_copy_of(const char *str);
string string_copy_of_len(const char *str, size_t len);

string string_dup(string s);

// Returns true if string equal to a string ending with 0.
bool string_eq_sz(string s, const char *str);

// Release string buffer.
void string_release(string s);

#ifdef __cplusplus
}
#endif

#endif
