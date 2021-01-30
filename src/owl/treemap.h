#ifndef OWL_TREEMAP_H
#define OWL_TREEMAP_H

#include "foundation/lang.h"
#include "owl/lkey.h"

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>

// AA tree implementation.

// Tree node. Tree key is a tuple (key:int, string). If key:int are equal,
// strings are compared with memcmp.
typedef struct TreeNode {
  LKey key;
  int level;
  struct TreeNode *child[2];
  long value[];
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
void *TreeMap_put(TreeMap *t, LKey key);
void *TreeMap_get(TreeMap *t, LKey key);
bool TreeMap_del(TreeMap *t, LKey key);

inline static size_t TreeMap_size(TreeMap *t) {
  return t->size;
}

#endif
