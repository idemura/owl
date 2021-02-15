#ifndef OWL_TREE_MAP_H
#define OWL_TREE_MAP_H

#include "foundation/lang.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// AA tree map implementation.

typedef struct {
    long nk;
    const char *sk;
} tree_key;

typedef long (*compare_keys_fn)(const tree_key *a, const tree_key *b);

inline static tree_key tree_key_number(long n)
{
    return (tree_key){.nk = n};
}

inline static tree_key tree_key_strn(const char *s, size_t s_len)
{
    return (tree_key){.nk = (long) s_len, .sk = s};
}

inline static tree_key tree_key_strz(const char *s)
{
    return tree_key_strn(s, strlen(s));
}

typedef struct tree_link {
    struct tree_link *child[2];
    int level;
} tree_link;

typedef struct tree_node {
    // Link must come first, becase we cast tree_link* <-> tree_node*.
    tree_link link;

    // Universal tree key
    tree_key key;

    // Inline value
    alignas(void *) char value[];
} tree_node;

typedef struct {
    tree_node *(*allocatez)(void *ctx, size_t size);
    void (*release)(void *ctx, tree_node *n);
} node_memmgr;

typedef struct {
    tree_node *root;
    tree_link empty;

    compare_keys_fn compare_keys;

    // Num of nodes in the tree
    size_t size;

    node_memmgr *nmm;
    void *nmm_ctx;
    size_t node_size;
} tree_map;

// Create a new instance of a map
tree_map tree_map_new(
        compare_keys_fn compare_keys, node_memmgr *nmm, void *nmm_ctx, size_t value_size);

void tree_map_destroy(tree_map *t);

inline static size_t tree_map_size(const tree_map *t)
{
    return t->size;
}

void *tree_map_put(tree_map *t, tree_key key);
void *tree_map_get(tree_map *t, tree_key key);
bool tree_map_del(tree_map *t, tree_key key);

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
