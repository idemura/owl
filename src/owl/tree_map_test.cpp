#include "owl/tree_map.h"

#include <gtest/gtest.h>

#include <iostream>

#define TREE_MAP_LEVEL(t, ...) TREE_MAP_PATH(t, __VA_ARGS__)->link.level
#define TREE_MAP_LONG_KEY(t, ...) TREE_MAP_PATH(t, __VA_ARGS__)->key.nk

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
    if (c->link.level == 0) {
        std::cerr << "null";
    } else {
        std::cerr << "key=" << c->key.nk << " level=" << c->link.level;
    }
}

static bool check_tree_map(const tree_map *t)
{
    const tree_node *e = tree_map_check(t);
    if (e) {
        std::cerr << "AA property violated:\n";
        std::cerr << "error at node key=" << e->key.nk << " level=" << e->link.level << "\n";

        std::cerr << "  ";
        print_child((tree_node *) e->link.child[0], "0");
        std::cerr << "\n";

        std::cerr << "  ";
        print_child((tree_node *) e->link.child[1], "1");
        std::cerr << "\n";

        if (e->link.child[1]->level != 0) {
            std::cerr << "  ";
            print_child((tree_node *) e->link.child[1]->child[1], "1-1");
            std::cerr << "\n";
        }
    }
    return e == nullptr;
}

static node_memmgr vtbl_nmm{.allocatez = allocatez, .release = release};

static tree_node *new_node(tree_map *t, long k, unsigned level)
{
    auto n = (tree_node *) vtbl_nmm.allocatez(t->nmm_ctx, sizeof(tree_node) + sizeof(int));
    n->key.nk = k;
    n->link.level = level;
    n->link.child[0] = &t->empty;
    n->link.child[1] = &t->empty;
    *((int *) n->value) = 0;
    return n;
}

TEST(tree_map, tree_map_put)
{
    node_memmgr_ctx nmm_ctx;
    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));
    EXPECT_EQ(0, t.size);

    *(int *) tree_map_put(&t, tree_key_number(300)) = 1;
    EXPECT_EQ(300, t.root->key.nk);
    EXPECT_EQ(1, t.root->link.level);
    EXPECT_EQ(0, t.root->link.child[0]->level);
    EXPECT_EQ(0, t.root->link.child[1]->level);
    EXPECT_TRUE(check_tree_map(&t));

    *(int *) tree_map_put(&t, tree_key_number(100)) = 2;
    EXPECT_EQ(1, TREE_MAP_LEVEL(&t));
    EXPECT_EQ(100, TREE_MAP_LONG_KEY(&t));
    EXPECT_EQ(0, TREE_MAP_LEVEL(&t, 0));
    EXPECT_EQ(1, TREE_MAP_LEVEL(&t, 1));
    EXPECT_EQ(300, TREE_MAP_LONG_KEY(&t, 1));
    EXPECT_TRUE(check_tree_map(&t));

    *(int *) tree_map_put(&t, tree_key_number(200)) = 3;
    EXPECT_EQ(2, TREE_MAP_LEVEL(&t));
    EXPECT_EQ(200, TREE_MAP_LONG_KEY(&t));
    EXPECT_EQ(1, TREE_MAP_LEVEL(&t, 0));
    EXPECT_EQ(100, TREE_MAP_LONG_KEY(&t, 0));
    EXPECT_EQ(1, TREE_MAP_LEVEL(&t, 1));
    EXPECT_EQ(300, TREE_MAP_LONG_KEY(&t, 1));
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
    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));

    tree_node *r = new_node(&t, 500, 2);
    r->link.child[0] = (tree_link *) new_node(&t, 250, 1);
    r->link.child[0]->child[1] = (tree_link *) new_node(&t, 300, 1);
    r->link.child[1] = (tree_link *) new_node(&t, 750, 2);
    r->link.child[1]->child[0] = (tree_link *) new_node(&t, 700, 1);
    r->link.child[1]->child[1] = (tree_link *) new_node(&t, 800, 1);

    t.root = r;
    t.size = 6;
    ASSERT_TRUE(check_tree_map(&t));

    tree_map_put(&t, tree_key_number(200));

    EXPECT_EQ(3, TREE_MAP_LEVEL(&t));
    EXPECT_EQ(500, TREE_MAP_LONG_KEY(&t));

    EXPECT_EQ(2, TREE_MAP_LEVEL(&t, 0));
    EXPECT_EQ(250, TREE_MAP_LONG_KEY(&t, 0));

    EXPECT_EQ(2, TREE_MAP_LEVEL(&t, 1));
    EXPECT_EQ(750, TREE_MAP_LONG_KEY(&t, 1));
}

TEST(tree_map, tree_map_del)
{
    // TODO
}
