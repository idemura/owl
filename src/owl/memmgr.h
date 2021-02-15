#ifndef OWL_MEMMGR_H
#define OWL_MEMMGR_H

#include "foundation/lang.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory manager/allcator.

typedef struct {
    // Allocate memory initialized with 0.
    void *(*allocatez)(void *ctx, size_t size);

    // Releases allocated memory (@ptr can be NULL).
    void (*release)(void *ctx, void *ptr);
} memmgr;

memmgr *get_std_memmgr(void);

#ifdef __cplusplus
}
#endif

#endif
