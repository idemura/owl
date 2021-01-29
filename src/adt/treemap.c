#include "adt/treemap.h"

static TreeNode empty;

void TreeMap_init(void) {
  empty.child[0] = &empty;
  empty.child[1] = &empty;
}

bool TreeMap_isNull(TreeNode *n) {
  return n == &empty;
}

void TreeMap_new(TreeMap *t, INodeMemMgr *nmm, size_t valueSize) {
  t->nmm = nmm;
  t->nodeSize = sizeof(TreeNode) + valueSize;
  t->size = 0;
  t->root = &empty;
}

inline static void *TreeMap_value(TreeNode *n) {
  return (char *) n + sizeof(TreeNode);
}

inline static void TreeMap_initNode(TreeNode *n, const Key *key) {
  n->key = *key;
  n->level = 1;
  n->child[0] = &empty;
  n->child[1] = &empty;
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
  const Key *key;
  TreeNode *res;
} PutState;

static TreeNode *TreeMap_putRec(PutState *state, TreeNode *node) {
  if (node == &empty) {
    TreeNode *p = state->t->nmm->allocatez(state->t->nodeSize);
    TreeMap_initNode(p, state->key);
    state->t->size++;
    state->res = p;
    return p;
  }
  int d = Key_compare(state->key, &node->key);
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

void *TreeMap_put(TreeMap *t, Key key) {
  PutState state = {.t = t, .key = &key};
  t->root = TreeMap_putRec(&state, t->root);
  return TreeMap_value(state.res);
}

void *TreeMap_get(TreeMap *t, Key key) {
  TreeNode *node = t->root;
  while (node != &empty) {
    int d = Key_compare(&key, &node->key);
    if (d == 0) {
      return TreeMap_value(node);
    }
    node = node->child[d > 0];
  }
  return NULL;
}

typedef struct {
  TreeMap *t;
  const Key *key;
} DelState;

static TreeNode *TreeMap_delRec(DelState *state, TreeNode *node) {
  if (node == &empty) {
    return node;
  }
  int d = Key_compare(state->key, &node->key);
  if (d == 0) {
    state->t->size--;
    TreeNode *r;
    if (node->child[0] == &empty) {
      r = node->child[1];
    } else {
      r = node;
    }
    return r;
  }
  int branch = d > 0;
  TreeNode *p = node;
  TreeNode *currentChild = p->child[branch];
  TreeNode *c = (p->child[branch] = TreeMap_delRec(state, currentChild));
  return c;
}

bool TreeMap_del(TreeMap *t, Key key) {
  size_t s = t->size;
  DelState state = {.t = t, .key = &key};
  t->root = TreeMap_delRec(&state, t->root);
  return t->size < s;
}
