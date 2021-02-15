#ifndef OWL_TREE_MAP_H
#define OWL_TREE_MAP_H

#include "foundation/lang.h"
#include "owl/memmgr.h"
#include "owl/skey.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tree map implementation based on AA tree map.

typedef struct tree_link {
    struct tree_link *child[2];
    int level;
} tree_link;

typedef struct tree_node {
    // Link must come first, becase we cast tree_link* <-> tree_node*.
    tree_link link;

    // Universal tree key
    skey_t key;

    // Inline value
    alignas(void *) char value[];
} tree_node;

typedef struct {
    tree_node *root;
    tree_link empty;

    skey_compare_fn compare_keys;

    // Num of nodes in the tree
    size_t size;

    // Node size in bytes with value
    size_t node_size;

    memmgr *mm;
    void *mm_ctx;
} tree_map;

// Create a new instance of a map
tree_map tree_map_new(skey_compare_fn compare_keys, memmgr *mm, void *mm_ctx, size_t value_size);

tree_map tree_map_clone(const tree_map *t);

void tree_map_destroy(tree_map *t);

inline static size_t tree_map_size(const tree_map *t)
{
    return t->size;
}

void *tree_map_put(tree_map *t, skey_t key);
void *tree_map_get(tree_map *t, skey_t key);
bool tree_map_del(tree_map *t, skey_t key);

const tree_node *tree_map_path(tree_map *t, int path_len, const int *path);

#define tree_map_path_va(t, ...) \
    ({ \
        const int path[] = {__VA_ARGS__}; \
        tree_map_path(t, array_sizeof(path), path); \
    })

// Check AA tree properties. Returns node where property is violated.
const tree_node *tree_map_check(const tree_map *t);

#ifdef __cplusplus
}
#endif

#endif
