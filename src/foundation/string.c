#include "foundation/string.h"

#include "foundation/memmgr.h"

char string_empty_buf[] = "";

string string_copy_of(const char *str)
{
    string s;
    s.len = strlen(str);
    s.str = memmgr_allocate_dirty(NULL, pad_size_l(s.len + 1));
    memcpy(s.str, str, s.len + 1);
    return s;
}

void string_release(string s)
{
    memmgr_release(NULL, s.str);
}
