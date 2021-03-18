#include "foundation/string.h"

#include "foundation/memmgr.h"

char string_empty_buf[sizeof(long)];

string string_copy_of(const char *str)
{
    return string_copy_of_len(str, strlen(str));
}

string string_copy_of_len(const char *str, size_t len)
{
    string s;
    s.len = len;
    assert(s.len < OWL_STRING_MAX);
    if (len == 0) {
        s.str = string_empty_buf;
    } else {
        size_t mem_size = pad_size_l(s.len + 1);
        s.str = memmgr_allocate_dirty(NULL, mem_size);

        // Copy string data
        memcpy(s.str, str, s.len);

        // Fill padding with zeroes
        for (size_t i = len; i < mem_size; i++) {
            s.str[i] = 0;
        }
    }
    return s;
}

string string_dup(string s)
{
    return string_copy_of_len(s.str, s.len);
}

void string_release(string s)
{
    memmgr_release(NULL, s.str);
}

bool string_eq_sz(string s, const char *str)
{
    size_t len = strlen(str);
    return s.len == len && memcmp(str, s.str, len) == 0;
}
