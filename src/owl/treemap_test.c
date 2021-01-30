#include "owl/treemap.h"

#include "testing/testing.h"

static size_t nNodes;

static TreeNode *allocatez(size_t size) {
  ++nNodes;
  return calloc(1, size);
}

static void release(TreeNode *n) {
  --nNodes;
  return free(n);
}

static INodeMemMgr nmm = {
    .allocatez = allocatez,
    .release = release,
};

static void test_TreeMap_put(void) {
  TreeMap t;
  TreeMap_new(&t, &nmm, sizeof(int));
  CHECK(t.size == 0);

  *(int *) TreeMap_put(&t, LKey_number(30)) = 1;
  CHECK(t.root->key.nk == 30);
  CHECK(t.root->level == 1);
  CHECK(TreeMap_isNull(t.root->child[0]));
  CHECK(TreeMap_isNull(t.root->child[1]));

  *(int *) TreeMap_put(&t, LKey_number(10)) = 2;
  CHECK(t.root->key.nk == 10);
  CHECK(t.root->level == 1);
  CHECK(TreeMap_isNull(t.root->child[0]));
  CHECK(t.root->child[1]->key.nk == 30);
  CHECK(t.root->child[1]->level == 1);

  *(int *) TreeMap_put(&t, LKey_number(20)) = 3;
  CHECK(t.root->key.nk == 20);
  CHECK(t.root->level == 2);
  CHECK(t.root->child[0]->key.nk == 10);
  CHECK(t.root->child[0]->level == 1);
  CHECK(t.root->child[1]->key.nk == 30);
  CHECK(t.root->child[1]->level == 1);

  CHECK(t.size == 3);

  CHECK(*(int *) TreeMap_get(&t, LKey_number(10)) == 2);
  CHECK(*(int *) TreeMap_get(&t, LKey_number(20)) == 3);
  CHECK(*(int *) TreeMap_get(&t, LKey_number(30)) == 1);

  TreeMap_destroy(&t);
  CHECK(nNodes == 0);
}

static void test_TreeMap_del(void) {
  // TODO
}

static void test_TreeMap_begin() {
  TreeMap_init();
}

static void test_TreeMap_end() {
  CHECK(nNodes == 0);
}

void test_TreeMap(void) {
  static TestCase tests[] = {
    TEST_CASE(test_TreeMap_put),
    TEST_CASE(test_TreeMap_del),
  };
  testingAdd(&test_TreeMap_begin, &test_TreeMap_end, tests, ARRAY_SIZE(tests));
}
