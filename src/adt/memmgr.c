#include "adt/memmgr.h"

#include <memory.h>
#include <stdlib.h>

static IMemMgr stdMemMgr = {
    .allocatez = calloc,
    .release = free,
};

IMemMgr *getStdMemMgr() {
  return &stdMemMgr;
}
