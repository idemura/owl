
#include "src/adt/treemap.h"

#include <stdlib.h>

#include "src/testing/testing.h"

static INodeMemMgr s_nmm;
static size_t nNodes;

static TreeNode *allocate(size_t size) {
  ++nNodes;
  return malloc(size);
}

static void release(TreeNode *n) {
  --nNodes;
  return free(n);
}

void TreeMap_test_begin() {
  s_nmm.allocate = &allocate;
  s_nmm.release = &release;
  TreeMap_init();
}

void TreeMap_test_end() {
  CHECK(nNodes == 0);
}

void TreeMap_test_put() {
  TreeMap t;
  TreeMap_new(&t, &s_nmm, sizeof(int));
  CHECK(t.size == 0);

  *(int *) TreeMap_put(&t, 30, NULL, 0, 0) = 1;
  *(int *) TreeMap_put(&t, 10, NULL, 0, 0) = 2;
  *(int *) TreeMap_put(&t, 20, NULL, 0, 0) = 3;

  CHECK(t.size == 3);

  CHECK(*(int *) TreeMap_get(&t, 10, NULL, 0) == 2);
  CHECK(*(int *) TreeMap_get(&t, 20, NULL, 0) == 3);
  CHECK(*(int *) TreeMap_get(&t, 30, NULL, 0) == 1);

  TreeMap_delete(&t);
  CHECK(nNodes == 0);
}
