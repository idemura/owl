#include "owl/tree_map.h"

#include <gtest/gtest.h>

#include <iostream>
#include <map>

#define GET_TREE_NODE_LEVEL(t, ...) tree_map_path_va(t, __VA_ARGS__)->link.level
#define GET_TREE_NODE_KEY(t, ...) tree_map_path_va(t, __VA_ARGS__)->key

struct node_memmgr_ctx {
    unsigned n_nodes = 0;
};

static tree_node *nmm_allocatez(void *ctx, size_t size)
{
    auto *test_ctx = (node_memmgr_ctx *) ctx;
    test_ctx->n_nodes++;
    return (tree_node *) calloc(1, size);
}

static void nmm_release(void *ctx, tree_node *n)
{
    auto *test_ctx = (node_memmgr_ctx *) ctx;
    test_ctx->n_nodes--;
    return free(n);
}

static node_memmgr vtbl_nmm{.allocatez = nmm_allocatez, .release = nmm_release};

static void print_child(const tree_node *c, const char *name)
{
    std::cerr << "child " << name << ": ";
    if (c->link.level == 0) {
        std::cerr << "null";
    } else {
        std::cerr << "key=" << c->key << " level=" << c->link.level;
    }
}

static bool check_tree_map(const tree_map *t)
{
    const tree_node *e = tree_map_check(t);
    if (e) {
        std::cerr << "AA property violated:\n";
        std::cerr << "error at node key=" << e->key << " level=" << e->link.level << "\n";

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

static tree_node *new_node(tree_map *t, long key, unsigned level)
{
    auto n = (tree_node *) vtbl_nmm.allocatez(t->nmm_ctx, sizeof(tree_node) + sizeof(int));
    n->key = key;
    n->link.level = level;
    n->link.child[0] = &t->empty;
    n->link.child[1] = &t->empty;
    *((int *) n->value) = 0;
    return n;
}

constexpr long N_A = 0;

struct node_proto {
    long key = N_A;
    unsigned level = 0;
    long lkey = N_A;
    long rkey = N_A;
};

static void construct_tree_nodes(tree_map *t, const node_proto *protos_p, size_t protos_n)
{
    std::map<tree_key, tree_node *> nodes;
    for (size_t i = 0; i < protos_n; i++) {
        nodes.emplace(protos_p[i].key, new_node(t, protos_p[i].key, protos_p[i].level));
    }
    for (size_t i = 0; i < protos_n; i++) {
        auto *n = nodes[protos_p[i].key];
        if (protos_p[i].lkey > 0) {
            n->link.child[0] = (tree_link *) nodes[protos_p[i].lkey];
        }
        if (protos_p[i].rkey) {
            n->link.child[1] = (tree_link *) nodes[protos_p[i].rkey];
        }
    }
    t->root = nodes[protos_p[0].key];
    t->size = protos_n;

    ASSERT_TRUE(check_tree_map(t));
}

TEST(tree_map, tree_map_put)
{
    node_memmgr_ctx nmm_ctx;
    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));
    EXPECT_EQ(0, t.size);

    *(int *) tree_map_put(&t, tree_key_number(300)) = 1;
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 1));
    ASSERT_TRUE(check_tree_map(&t));

    *(int *) tree_map_put(&t, tree_key_number(100)) = 2;
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(100, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 1));
    ASSERT_TRUE(check_tree_map(&t));

    *(int *) tree_map_put(&t, tree_key_number(200)) = 3;
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(200, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(100, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 1));
    ASSERT_TRUE(check_tree_map(&t));

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

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1, N_A, 300},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    construct_tree_nodes(&t, protos, array_sizeof(protos));

    tree_map_put(&t, tree_key_number(200));

    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t));

    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t, 0));

    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, nmm_ctx.n_nodes);
}

TEST(tree_map, tree_map_del_h1_c0)
{
    node_memmgr_ctx nmm_ctx;
    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_nodes(&t, protos, array_sizeof(protos));

    tree_map_del(&t, tree_key_number(250));

    EXPECT_EQ(2, t.size);
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, nmm_ctx.n_nodes);
}

TEST(tree_map, tree_map_del_h1_c1)
{
    node_memmgr_ctx nmm_ctx;
    tree_map t = tree_map_new(&vtbl_nmm, &nmm_ctx, sizeof(int));

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_nodes(&t, protos, array_sizeof(protos));

    tree_map_del(&t, tree_key_number(750));

    EXPECT_EQ(2, t.size);
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, nmm_ctx.n_nodes);
}
