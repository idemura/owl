#include "owl/treemap.h"

#include <gtest/gtest.h>

struct test_node_memmgr_ctx {
    unsigned int n_nodes;
};

static tree_node *allocatez(void *ctx, size_t size)
{
    auto *test_ctx = (test_node_memmgr_ctx *) ctx;
    test_ctx->n_nodes++;
    return (tree_node *) calloc(1, size);
}

static void release(void *ctx, tree_node *n)
{
    auto *test_ctx = (test_node_memmgr_ctx *) ctx;
    test_ctx->n_nodes--;
    return free(n);
}

TEST(tree_map, treemap_put)
{
    test_node_memmgr_ctx nmm_ctx{0};
    node_memmgr nmm{.allocatez = allocatez, .release = release};

    treemap t = treemap_new(&nmm, &nmm_ctx, sizeof(int));
    EXPECT_EQ(0, t.size);

    *(int *) treemap_put(&t, lkey_number(30)) = 1;
    EXPECT_EQ(30, t.root->key.nk);
    EXPECT_EQ(1, t.root->level);
    EXPECT_TRUE(treemap_is_null(t.root->child[0]));
    EXPECT_TRUE(treemap_is_null(t.root->child[1]));

    *(int *) treemap_put(&t, lkey_number(10)) = 2;
    EXPECT_EQ(10, t.root->key.nk);
    EXPECT_EQ(1, t.root->level);
    EXPECT_TRUE(treemap_is_null(t.root->child[0]));
    EXPECT_EQ(30, t.root->child[1]->key.nk);
    EXPECT_EQ(1, t.root->child[1]->level);

    *(int *) treemap_put(&t, lkey_number(20)) = 3;
    EXPECT_EQ(20, t.root->key.nk);
    EXPECT_EQ(2, t.root->level);
    EXPECT_EQ(10, t.root->child[0]->key.nk);
    EXPECT_EQ(1, t.root->child[0]->level);
    EXPECT_EQ(30, t.root->child[1]->key.nk);
    EXPECT_EQ(1, t.root->child[1]->level);

    EXPECT_EQ(3, t.size);

    EXPECT_EQ(2, *(int *) treemap_get(&t, lkey_number(10)));
    EXPECT_EQ(3, *(int *) treemap_get(&t, lkey_number(20)));
    EXPECT_EQ(1, *(int *) treemap_get(&t, lkey_number(30)));

    treemap_destroy(&t);
    EXPECT_EQ(0, nmm_ctx.n_nodes);
}

TEST(tree_map, treemap_delete)
{
    // TODO
}
