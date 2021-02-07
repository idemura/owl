#include "owl/tree_map.h"

#include <gtest/gtest.h>

#include <iostream>

struct node_memmgr_ctx {
    unsigned n_nodes = 0;
};

static tree_node *allocatez(void *ctx, size_t size)
{
    auto *test_ctx = (node_memmgr_ctx *) ctx;
    test_ctx->n_nodes++;
    return (tree_node *) calloc(1, size);
}

static void release(void *ctx, tree_node *n)
{
    auto *test_ctx = (node_memmgr_ctx *) ctx;
    test_ctx->n_nodes--;
    return free(n);
}

static void print_child(const tree_node *c, const char *name)
{
    std::cerr << "child " << name << ": ";
    if (tree_map_is_null(c)) {
        std::cerr << "null";
    } else {
        std::cerr << "key=" << c->key.nk << " level=" << c->level;
    }
}

static bool check_tree_map(const tree_map *t)
{
    const tree_node *e = tree_map_check(t);
    if (e) {
        std::cerr << "AA property violated:\n";
        std::cerr << "error at node key=" << e->key.nk << " level=" << e->level << "\n";

        std::cerr << "  ";
        print_child(e->child[0], "0");
        std::cerr << "\n";

        std::cerr << "  ";
        print_child(e->child[1], "1");
        std::cerr << "\n";

        if (!tree_map_is_null(e->child[1])) {
            std::cerr << "  ";
            print_child(e->child[1]->child[1], "1-1");
            std::cerr << "\n";
        }
    }
    return e == nullptr;
}

static node_memmgr vtbl_nmm{.allocatez = allocatez, .release = release};

static tree_node *new_node(node_memmgr_ctx *ctx, long k, unsigned level)
{
    auto n = (tree_node *) vtbl_nmm.allocatez(ctx, sizeof(tree_node) + sizeof(int));
    n->key.nk = k;
    n->level = level;
    n->child[0] = tree_map_empty();
    n->child[1] = tree_map_empty();
    *((int*)n->value) = 0;
    return n;
}

TEST(tree_map, tree_map_put)
{
    node_memmgr_ctx nmm_ctx;
    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));
    EXPECT_EQ(0, t.size);

    *(int *) tree_map_put(&t, tree_key_number(300)) = 1;
    EXPECT_EQ(300, t.root->key.nk);
    EXPECT_EQ(1, t.root->level);
    EXPECT_TRUE(tree_map_is_null(t.root->child[0]));
    EXPECT_TRUE(tree_map_is_null(t.root->child[1]));
    EXPECT_TRUE(check_tree_map(&t));

    *(int *) tree_map_put(&t, tree_key_number(100)) = 2;
    EXPECT_EQ(100, t.root->key.nk);
    EXPECT_EQ(1, t.root->level);
    EXPECT_TRUE(tree_map_is_null(t.root->child[0]));
    EXPECT_EQ(300, t.root->child[1]->key.nk);
    EXPECT_EQ(1, t.root->child[1]->level);
    EXPECT_TRUE(check_tree_map(&t));

    *(int *) tree_map_put(&t, tree_key_number(200)) = 3;
    EXPECT_EQ(200, t.root->key.nk);
    EXPECT_EQ(2, t.root->level);
    EXPECT_EQ(100, t.root->child[0]->key.nk);
    EXPECT_EQ(1, t.root->child[0]->level);
    EXPECT_EQ(300, t.root->child[1]->key.nk);
    EXPECT_EQ(1, t.root->child[1]->level);
    EXPECT_TRUE(check_tree_map(&t));

    EXPECT_EQ(3, t.size);

    EXPECT_EQ(2, *(int *) tree_map_get(&t, tree_key_number(100)));
    EXPECT_EQ(3, *(int *) tree_map_get(&t, tree_key_number(200)));
    EXPECT_EQ(1, *(int *) tree_map_get(&t, tree_key_number(300)));

    tree_map_destroy(&t);
    EXPECT_EQ(0, nmm_ctx.n_nodes);
}

TEST(tree_map, tree_map_put_skew_split)
{
    node_memmgr_ctx nmm_ctx;

    tree_node *r = new_node(&nmm_ctx, 500, 2);
    r->child[0] = new_node(&nmm_ctx, 250, 1);
    r->child[0]->child[1] = new_node(&nmm_ctx, 300, 1);
    r->child[1] = new_node(&nmm_ctx, 750, 2);
    r->child[1]->child[0] = new_node(&nmm_ctx, 700, 1);
    r->child[1]->child[1] = new_node(&nmm_ctx, 800, 1);

    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));
    t.root = r;
    t.size = 6;
    ASSERT_TRUE(check_tree_map(&t));

    tree_map_put(&t, tree_key_number(200));

    const tree_node *p;

    p = tree_map_path(&t, 0);
    EXPECT_EQ(500, p->key.nk);
    EXPECT_EQ(3, p->level);
}

TEST(tree_map, tree_map_del)
{
    // TODO
}
