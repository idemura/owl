#ifndef FOUNDATION_TREE_MAP_H
#define FOUNDATION_TREE_MAP_H

#include "foundation/lang.h"
#include "foundation/memmgr.h"
#include "foundation/skey.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Tree map implementation based on AA tree map.
 */

typedef struct tree_node {
    struct tree_node *child[2];

    // AA tree node level (0 for spcial "empty" node).
    size_t level;

    alignas(long) unsigned char value[];
} tree_node;

typedef struct {
    tree_node *root;

    skey_compare_fn compare_keys;

    // Value size in bytes with value
    size_t value_size;

    // Number of nodes in the tree
    size_t size;

    memmgr_ctx *mmc;

    tree_node empty;
} tree_map;

// Create a new instance of a map
tree_map tree_map_new(skey_compare_fn compare_keys, memmgr_ctx *mmc, size_t value_size);

inline static tree_map tree_map_new_default(skey_compare_fn compare_keys, size_t value_size)
{
    return tree_map_new(compare_keys, NULL, value_size);
}

tree_map tree_map_clone(const tree_map *t);

// Check AA tree properties. Returns node where property is violated.
const tree_node *tree_map_check(const tree_map *t);

void tree_map_destroy(tree_map *t);

inline static size_t tree_map_size(const tree_map *t)
{
    return t->size;
}

void tree_map_put(tree_map *t, skey_t key_value);

#define tree_map_put_v(t, key_value) \
    ({ \
        __typeof__(key_value) lvalue = (key_value); \
        tree_map_put((t), SKEY_OF(&lvalue)); \
    })

void *tree_map_get(tree_map *t, skey_t key);

#define tree_map_get_v(t, key) \
    ({ \
        __typeof__(key) lvalue = (key); \
        tree_map_get((t), SKEY_OF(&lvalue)); \
    })

void tree_map_del(tree_map *t, skey_t key);

#define tree_map_del_v(t, key) \
    ({ \
        __typeof__(key) lvalue = (key); \
        tree_map_del((t), SKEY_OF(&lvalue)); \
    })

const tree_node *tree_map_path(tree_map *t, int path_len, const int *path);

#define tree_map_path_va(t, ...) \
    ({ \
        const int path[] = {__VA_ARGS__}; \
        tree_map_path((t), array_sizeof(path), path); \
    })

void *tree_map_min_key(tree_map *t);
void *tree_map_max_key(tree_map *t);

typedef struct {
    // Iterator direction: 0 - forward, 1 - backward.
    int dir;

    tree_node *empty;

    // Size of the stack
    size_t top;

    // We need to keep track of left branch. Because left child's level if one less than parent,
    // given max size is OWL_MAX_SIZE = 2**48 - 1, we only need to have stack of size:
    //   log_2(OWL_MAX_SIZE + 1) = 48.
    //
    // Node on the stack means that node's right subtree and the node itself is not visited.
    tree_node *stack[48];
} tree_map_iter;

// Init forward/backward iterator at the first element in that direction.
void *tree_map_begin(tree_map *t, tree_map_iter *iter, bool fwd);

// Init iterator at a certain key (or greater).
void *tree_map_begin_at(tree_map *t, tree_map_iter *iter, bool fwd, skey_t key);

#define tree_map_begin_at_v(t, iter, fwd, key) \
    ({ \
        __typeof__(key) lvalue = (key); \
        tree_map_begin_at((t), (iter), (fwd), SKEY_OF(&lvalue)); \
    })

// Get next, or null if itreation complete.
void *tree_map_iter_next(tree_map_iter *iter);

#ifdef __cplusplus
}
#endif

#endif
