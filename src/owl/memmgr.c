#include "owl/memmgr.h"

#include <memory.h>
#include <stdlib.h>

static memmgr s_mm = {
		.allocatez = calloc,
		.release = free,
};

memmgr *std_mm = &s_mm;
