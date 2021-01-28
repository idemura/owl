#include "adt/memmgr.h"

#include <memory.h>
#include <stdlib.h>

static void *mallocZero(size_t size) {
  void *p = malloc(size);
  return memset(p, 0, size);
}

static IMemMgr stdMemMgr = {
    .allocate = mallocZero,
    .release = free,
};

IMemMgr *getStdMemMgr() {
  return &stdMemMgr;
}
