#ifndef OWL_MEMMGR_H
#define OWL_MEMMGR_H

#include "foundation/lang.h"

#define MM_ALLOCATEZ(n, size) std_mm->allocatez(n, size)
#define MM_RELEASE(p) std_mm->release(p)

typedef struct {
    // Allocate and clear memory with 0
    void *(*allocatez)(size_t n, size_t size);
    void (*release)(void *p);
} memmgr;

extern memmgr *std_mm;

#endif
