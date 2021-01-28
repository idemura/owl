#ifndef ADT_MEMMGR_H
#define ADT_MEMMGR_H

#include "types/primitive.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  // Allocate and clear memory with 0
  void *(*allocatez)(size_t n, size_t size);
  void (*release)(void *p);
} IMemMgr;

IMemMgr *getStdMemMgr();

#ifdef __cplusplus
}
#endif

#endif
