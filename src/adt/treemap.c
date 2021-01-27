#include "src/adt/treemap.h"

static TreeNode s_none;

void TreeMap_init() {
  s_none.child[0] = &s_none;
  s_none.child[1] = &s_none;
}

void TreeMap_new(TreeMap *t, INodeMemMgr *nmm, size_t valueSize) {
  t->nmm = nmm;
  t->valueSize = valueSize;
  t->size = 0;
  t->root = &s_none;
}

inline static void *TreeMap_value(TreeNode *n) {
  return (char *) n + sizeof(TreeNode);
}

inline static void TreeMap_initNode(TreeNode *n, i64 keyInt, char const *keyStr, u32 keyStrLength) {
  n->keyInt = keyInt;
  n->keyStr = keyStr;
  n->keyStrLength = keyStrLength;
  n->level = 0;
  n->child[0] = &s_none;
  n->child[1] = &s_none;
}

inline static int TreeMap_compare(TreeNode const *a, TreeNode const *b) {
  if (a->keyInt == b->keyInt) {
    if (b->keyStrLength == 0) {
      return a->keyStrLength != 0;
    }
    if (a->keyStrLength == 0) {
      return -1;
    }
    size_t cmpBytes = a->keyStrLength < b->keyStrLength ? a->keyStrLength : b->keyStrLength;
    int d = memcmp(a->keyStr, b->keyStr, cmpBytes);
    if (d == 0) {
      return (int) (a->keyStrLength - b->keyStrLength);
    }
    return d;
  } else {
    return (int) (a->keyInt - b->keyInt);
  }
}

static void TreeMap_deleteNodeRec(INodeMemMgr *nmm, TreeNode *node) {
  if (node != &s_none) {
    TreeMap_deleteNodeRec(nmm, node->child[0]);
    TreeMap_deleteNodeRec(nmm, node->child[1]);
    nmm->release(node);
  }
}

void TreeMap_delete(TreeMap *t) {
  TreeMap_deleteNodeRec(t->nmm, t->root);
}

typedef struct {
  TreeMap *t;
  TreeNode *result;
  size_t valueSize;
  TreeNode new;
} PutState;

static TreeNode *TreeMap_putRec(PutState *state, TreeNode *node) {
  if (node == &s_none) {
    TreeNode *p = state->t->nmm->allocate(sizeof(TreeNode) + state->valueSize);
    memcpy(p, &state->new, sizeof(TreeNode));
    state->t->size++;
    state->result = p;
    return p;
  }
  int d = TreeMap_compare(&state->new, node);
  if (d == 0) {
    state->result = node;
    return node;
  }
  int b = d > 0;
  node->child[b] = TreeMap_putRec(state, node->child[b]);
  return node;
}

void *TreeMap_put(TreeMap *t, i64 keyInt, char const *keyStr, u32 keyStrLength, size_t valueSize) {
  PutState state = {.t = t, .result = NULL, .valueSize = t->valueSize + valueSize};
  TreeMap_initNode(&state.new, keyInt, keyStr, keyStrLength);
  t->root = TreeMap_putRec(&state, t->root);
  return TreeMap_value(state.result);
}

void *TreeMap_get(TreeMap *t, i64 keyInt, char const *keyStr, u32 keyStrLength) {
  TreeNode target;
  TreeMap_initNode(&target, keyInt, keyStr, keyStrLength);
  TreeNode *node = t->root;
  while (node != &s_none) {
    int d = TreeMap_compare(&target, node);
    if (d == 0) {
      return TreeMap_value(node);
    }
    int b = d > 0;
    node = node->child[b];
  }
  return NULL;
}
