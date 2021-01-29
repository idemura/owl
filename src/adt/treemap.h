#ifndef ADT_TREEMAP_H
#define ADT_TREEMAP_H

#include "adt/key.h"
#include "types/primitive.h"

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>

// AA tree implementation.

// Tree node. Tree key is a tuple (key:int, string). If key:int are equal,
// strings are compared with memcmp.
typedef struct TreeNode {
  Key key;
  int level;
  struct TreeNode *child[2];
} TreeNode;

typedef struct {
  TreeNode *(*allocatez)(size_t size);
  void (*release)(TreeNode *n);
} INodeMemMgr;

// AA tree object.
typedef struct {
  INodeMemMgr *nmm;
  size_t nodeSize;
  size_t size;
  TreeNode *root;
} TreeMap;

void TreeMap_init(void);
bool TreeMap_isNull(TreeNode *n);
void TreeMap_new(TreeMap *t, INodeMemMgr *nmm, size_t valueSize);
void TreeMap_destroy();
void *TreeMap_put(TreeMap *t, Key key);
void *TreeMap_get(TreeMap *t, Key key);
bool TreeMap_del(TreeMap *t, Key key);

inline static size_t TreeMap_size(TreeMap *t) {
  return t->size;
}

#endif
