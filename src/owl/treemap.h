#ifndef OWL_TREEMAP_H
#define OWL_TREEMAP_H

#include "foundation/lang.h"
#include "owl/lkey.h"

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// AA tree implementation.

typedef struct tree_node {
    lkey key;
    int level;
    struct tree_node *child[2];
    long value[];
} tree_node;

typedef struct {
    tree_node *(*allocatez)(void *ctx, size_t size);
    void (*release)(void *ctx, tree_node *n);
} node_memmgr;

typedef struct {
    node_memmgr *nmm;
    void *nmm_ctx;
    size_t node_size;
    size_t size;
    tree_node *root;
} treemap;

void treemap_init(void);
bool treemap_is_null(tree_node *n);
treemap treemap_new(node_memmgr *nmm, void *ctx, size_t value_size);
void treemap_destroy(treemap *t);
void *treemap_put(treemap *t, lkey key);
void *treemap_get(treemap *t, lkey key);
bool treemap_delete(treemap *t, lkey key);

inline static size_t treemap_size(treemap *t)
{
    return t->size;
}

#ifdef __cplusplus
}
#endif

#endif
