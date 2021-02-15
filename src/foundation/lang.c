#include "foundation/lang.h"

static_assert(sizeof(long) == 8, "64 bit system assumed");

bool str_starts_with(const char *s, const char *pattern)
{
    return memcmp(s, pattern, strlen(pattern)) == 0;
}
