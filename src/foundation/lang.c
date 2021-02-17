#include "foundation/lang.h"

#include <stdio.h>

static_assert(sizeof(long) == 8, "64 bit system required");

noreturn void die(const char *format, ...)
{
    fprintf(stderr, "Fatal error: ");

    va_list va;
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);

    fprintf(stderr, "\n");

    exit(1);
}

bool str_starts_with(const char *s, const char *pattern)
{
    return memcmp(s, pattern, strlen(pattern)) == 0;
}
