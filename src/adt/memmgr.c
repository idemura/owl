#include "adt/memmgr.h"

#include <memory.h>
#include <stdlib.h>

static IMemMgr s_mm = {
    .allocatez = calloc,
    .release = free,
};

IMemMgr *stdmm = &s_mm;
