#ifndef ADT_TREEMAP_H_
#define ADT_TREEMAP_H_

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>

#include "types/primitive.h"

#ifdef __cplusplus
extern "C" {
#endif

// AA tree implementation.

// Tree node. Tree key is a tuple (key:int, string). If key:int are equal, strings are compared
// with memcmp.
typedef struct TreeNode {
  i64 keyInt;
  char const *keyStr;
  u32 keyStrLength;
  u32 level;
  struct TreeNode *child[2];
} TreeNode;

typedef struct {
  TreeNode *(*allocate)(size_t valueSize);
  void (*release)(TreeNode *node);
} INodeMemMgr;

// AA tree object.
typedef struct {
  INodeMemMgr *nmm;
  size_t valueSize;
  size_t size;
  TreeNode *root;
} TreeMap;

void TreeMap_init();
bool TreeMap_isNull(TreeNode *n);
void TreeMap_new(TreeMap *t, INodeMemMgr *nmm, size_t valueSize);
void TreeMap_delete();
void *TreeMap_put(TreeMap *t, i64 keyInt, char const *keyStr, u32 keyStrLength, size_t valueSize);
void *TreeMap_get(TreeMap *t, i64 keyInt, char const *keyStr, u32 keyStrLength);

inline size_t TreeMap_size(TreeMap *t) {
  return t->size;
}

#ifdef __cplusplus
}
#endif

#endif
