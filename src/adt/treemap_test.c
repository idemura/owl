#include "adt/treemap.h"

#include "testing/testing.h"

static INodeMemMgr nmm;
static size_t nNodes;

static TreeNode *allocatez(size_t size) {
  ++nNodes;
  return calloc(1, size);
}

static void release(TreeNode *n) {
  --nNodes;
  return free(n);
}

void test_TreeMap_begin() {
  nmm.allocatez = &allocatez;
  nmm.release = &release;
  TreeMap_init();
}

void test_TreeMap_end() {
  CHECK(nNodes == 0);
}

void test_TreeMap_put() {
  TreeMap t;
  TreeMap_new(&t, &nmm, sizeof(int));
  CHECK(t.size == 0);

  *(int *) TreeMap_put(&t, 30, NULL, 0, 0) = 1;
  CHECK(t.root->keyInt == 30);
  CHECK(t.root->level == 1);
  CHECK(TreeMap_isNull(t.root->child[0]));
  CHECK(TreeMap_isNull(t.root->child[1]));

  *(int *) TreeMap_put(&t, 10, NULL, 0, 0) = 2;
  CHECK(t.root->keyInt == 10);
  CHECK(t.root->level == 1);
  CHECK(TreeMap_isNull(t.root->child[0]));
  CHECK(t.root->child[1]->keyInt == 30);
  CHECK(t.root->child[1]->level == 1);

  *(int *) TreeMap_put(&t, 20, NULL, 0, 0) = 3;

  CHECK(t.size == 3);

  CHECK(*(int *) TreeMap_get(&t, 10, NULL, 0) == 2);
  CHECK(*(int *) TreeMap_get(&t, 20, NULL, 0) == 3);
  CHECK(*(int *) TreeMap_get(&t, 30, NULL, 0) == 1);

  TreeMap_destroy(&t);
  CHECK(nNodes == 0);
}
