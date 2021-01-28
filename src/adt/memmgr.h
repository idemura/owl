#ifndef ADT_MEMMGR_H_
#define ADT_MEMMGR_H_

#include "types/primitive.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  void *(*allocate)(size_t size);
  void (*release)();
} IMemMgr;

IMemMgr *getStdMemMgr();

#ifdef __cplusplus
}
#endif

#endif
