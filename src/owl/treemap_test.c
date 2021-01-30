#include "owl/treemap.h"

#include "testing/testing.h"

static size_t n_nodes;

static tree_node *allocatez(size_t size)
{
    ++n_nodes;
    return calloc(1, size);
}

static void release(tree_node *n)
{
    --n_nodes;
    return free(n);
}

static node_memmgr nmm = {
        .allocatez = allocatez,
        .release = release,
};

static void test_treemap_put(void)
{
    treemap t;
    treemap_new(&t, &nmm, sizeof(int));
    CHECK(t.size == 0);

    *(int *) treemap_put(&t, lkey_number(30)) = 1;
    CHECK(t.root->key.nk == 30);
    CHECK(t.root->level == 1);
    CHECK(treemap_is_null(t.root->child[0]));
    CHECK(treemap_is_null(t.root->child[1]));

    *(int *) treemap_put(&t, lkey_number(10)) = 2;
    CHECK(t.root->key.nk == 10);
    CHECK(t.root->level == 1);
    CHECK(treemap_is_null(t.root->child[0]));
    CHECK(t.root->child[1]->key.nk == 30);
    CHECK(t.root->child[1]->level == 1);

    *(int *) treemap_put(&t, lkey_number(20)) = 3;
    CHECK(t.root->key.nk == 20);
    CHECK(t.root->level == 2);
    CHECK(t.root->child[0]->key.nk == 10);
    CHECK(t.root->child[0]->level == 1);
    CHECK(t.root->child[1]->key.nk == 30);
    CHECK(t.root->child[1]->level == 1);

    CHECK(t.size == 3);

    CHECK(*(int *) treemap_get(&t, lkey_number(10)) == 2);
    CHECK(*(int *) treemap_get(&t, lkey_number(20)) == 3);
    CHECK(*(int *) treemap_get(&t, lkey_number(30)) == 1);

    treemap_destroy(&t);
    CHECK(n_nodes == 0);
}

static void test_treemap_del(void)
{
    // TODO
}

static void test_treemap_begin()
{
    treemap_init();
}

static void test_treemap_end()
{
    CHECK(n_nodes == 0);
}

void test_treemap(void)
{
    static test_case tests[] = {
            TEST_CASE(test_treemap_put),
            TEST_CASE(test_treemap_del),
    };
    testing_add(&test_treemap_begin, &test_treemap_end, tests, ARRAY_SIZE(tests));
}
