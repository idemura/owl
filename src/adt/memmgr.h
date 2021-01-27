#ifndef ADT_MEMMGR_H_
#define ADT_MEMMGR_H_

#include "src/types/ints.h"

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
