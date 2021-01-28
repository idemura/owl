#include "adt/treemap.h"

static TreeNode empty;

void TreeMap_init() {
  empty.child[0] = &empty;
  empty.child[1] = &empty;
}

bool TreeMap_isNull(TreeNode *n) {
  return n == &empty;
}

void TreeMap_new(TreeMap *t, INodeMemMgr *nmm, size_t valueSize) {
  t->nmm = nmm;
  t->valueSize = valueSize;
  t->size = 0;
  t->root = &empty;
}

inline static void *TreeMap_value(TreeNode *n) {
  return (char *) n + sizeof(TreeNode);
}

inline static void TreeMap_initNode(
    TreeNode *n, i64 keyInt, char const *keyStr, u32 keyStrLength) {
  n->keyInt = keyInt;
  n->keyStr = keyStr;
  n->keyStrLength = keyStrLength;
  n->level = 0;
  n->child[0] = &empty;
  n->child[1] = &empty;
}

inline static int TreeMap_compare(TreeNode const *a, TreeNode const *b) {
  if (!(a->keyStrLength | b->keyStrLength) || a->keyInt != b->keyInt) {
    return (int) (a->keyInt - b->keyInt);
  }
  if (!b->keyStrLength) {
    return 1;
  }
  if (!a->keyStrLength) {
    return -1;
  }
  size_t cmpBytes =
      a->keyStrLength < b->keyStrLength ? a->keyStrLength : b->keyStrLength;
  int d = memcmp(a->keyStr, b->keyStr, cmpBytes);
  if (d != 0) {
    return d;
  }
  return (int) (a->keyStrLength - b->keyStrLength);
}

static void TreeMap_destroyRec(INodeMemMgr *nmm, TreeNode *node) {
  if (node != &empty) {
    TreeMap_destroyRec(nmm, node->child[0]);
    TreeMap_destroyRec(nmm, node->child[1]);
    nmm->release(node);
  }
}

void TreeMap_destroy(TreeMap *t) {
  TreeMap_destroyRec(t->nmm, t->root);
}

typedef struct {
  TreeMap *t;
  TreeNode *key;
  TreeNode *res;
  size_t valueSize;
} PutState;

static TreeNode *TreeMap_putRec(PutState *state, TreeNode *node) {
  if (node == &empty) {
    TreeNode *p = state->t->nmm->allocate(sizeof(TreeNode) + state->valueSize);
    memcpy(p, state->key, sizeof(TreeNode));
    p->level = 1;
    state->t->size++;
    state->res = p;
    return p;
  }
  int d = TreeMap_compare(state->key, node);
  if (d == 0) {
    state->res = node;
    return node;
  }
  int branch = d > 0;
  TreeNode *p = node;
  TreeNode *currentChild = p->child[branch];
  TreeNode *c = (p->child[branch] = TreeMap_putRec(state, currentChild));
  if (c == currentChild) {
    // Fast return: do not touch other nodes (possibly not in cache)
    return node;
  }
  if (branch) {
    // Right child: test if we need to increment level first
    if (p->level < c->level) {
      p->level++;
    }
  } else if (p->level == c->level) {
    // Rotate right
    p->child[0] = c->child[1];
    c->child[1] = p;
    p = c;
  }
  // Check if we have 3 nodes of the same level of the right
  if (p->level == p->child[1]->child[1]->level) {
    c = p->child[1];
    p->child[1] = c->child[0];
    c->child[0] = p;
    c->level++;
    p = c;
  }
  return p;
}

void *TreeMap_put(
    TreeMap *t,
    i64 keyInt,
    char const *keyStr,
    u32 keyStrLength,
    size_t valueSize) {
  TreeNode k;
  TreeMap_initNode(&k, keyInt, keyStr, keyStrLength);
  PutState state = {
      .t = t, .key = &k, .res = NULL, .valueSize = t->valueSize + valueSize};
  t->root = TreeMap_putRec(&state, t->root);
  return TreeMap_value(state.res);
}

void *TreeMap_get(
    TreeMap *t, i64 keyInt, char const *keyStr, u32 keyStrLength) {
  TreeNode target;
  TreeMap_initNode(&target, keyInt, keyStr, keyStrLength);
  TreeNode *node = t->root;
  while (node != &empty) {
    int d = TreeMap_compare(&target, node);
    if (d == 0) {
      return TreeMap_value(node);
    }
    node = node->child[d > 0];
  }
  return NULL;
}

typedef struct {
  TreeMap *t;
  TreeNode *key;
} DelState;

static TreeNode *TreeMap_delRec(DelState *state, TreeNode *node) {
  if (node == &empty) {
    return node;
  }
  int d = TreeMap_compare(state->key, node);
  if (d == 0) {
    state->t->size--;
    return node;
  }
  int branch = d > 0;
  TreeNode *p = node;
  TreeNode *currentChild = p->child[branch];
  TreeNode *c = (p->child[branch] = TreeMap_delRec(state, currentChild));
  return c;
}

bool TreeMap_del(TreeMap *t, i64 keyInt, char const *keyStr, u32 keyStrLength) {
  size_t s = t->size;
  TreeNode k;
  TreeMap_initNode(&k, keyInt, keyStr, keyStrLength);
  DelState state = {.t = t, .key = &k};
  t->root = TreeMap_delRec(&state, t->root);
  return t->size < s;
}
