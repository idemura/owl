#include "foundation/tree_map.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <map>
#include <string>

#define GET_TREE_NODE_LEVEL(t, ...) tree_map_path_va(t, __VA_ARGS__)->level
#define GET_TREE_NODE_KEY(t, ...) *((const int *) tree_map_path_va(t, __VA_ARGS__)->value)

struct key_value {
    int k = 0;
    int v = 0;

    key_value(int k_, int v_): k{k_}, v{v_} {}
};

static long skey_compare(skey_t a, skey_t b)
{
    return *((const int *) a.ptr) - *((const int *) b.ptr);
}

static uint64_t skey_hash(skey_t a)
{
    return *((const int *) a.ptr);
}

static std::string to_string(skey_t key)
{
    return std::to_string(*(const int *) key.ptr);
}

static void print_child(std::ostream &os, const tree_node *c, const char *name)
{
    os << "child " << name << ": ";
    if (c->level == 0) {
        os << "null";
    } else {
        os << "key=" << to_string(SKEY_OF(c->value)) << " level=" << c->level;
    }
}

static bool check_tree(const tree_map &t, std::ostream &os)
{
    const tree_node *e = tree_map_check(&t);
    if (e) {
        os << "AA property violated:\n";
        os << "at node key=" << to_string(SKEY_OF(e->value)) << " level=" << e->level << "\n";

        os << "  ";
        print_child(os, (tree_node *) e->child[0], "0");
        os << "\n";

        os << "  ";
        print_child(os, (tree_node *) e->child[1], "1");
        os << "\n";

        if (e->child[1]->level != 0) {
            os << "  ";
            print_child(os, (tree_node *) e->child[1]->child[1], "1-1");
            os << "\n";
        }
    }
    return e == nullptr;
}

static bool check_tree(const tree_map &t)
{
    return check_tree(t, std::cerr);
}

static void print_tree_rec(const tree_map &t, std::ostream &os, const tree_node *node, int indent)
{
    for (int i = 0; i < indent; i++) {
        os << "    ";
    }
    if ((const tree_node *) node == &t.empty) {
        os << "(null)\n";
        return;
    }
    os << to_string(SKEY_OF(node->value)) << " level=" << node->level << "\n";
    if (node->child[0] == &t.empty && node->child[1] == &t.empty) {
        return;
    }
    print_tree_rec(t, os, (const tree_node *) node->child[0], indent + 1);
    print_tree_rec(t, os, (const tree_node *) node->child[1], indent + 1);
}

static void print_tree(const tree_map &t, std::ostream &os)
{
    if ((const tree_node *) t.root == &t.empty) {
        os << "empty\n";
    } else {
        print_tree_rec(t, os, t.root, 0);
    }
}

static tree_node *new_node(tree_map *t, key_value kv, int level)
{
    auto n = (tree_node *) memmgr_allocate_clear(t->mmc, sizeof(tree_node) + sizeof(key_value));
    n->level = level;
    n->child[0] = &t->empty;
    n->child[1] = &t->empty;
    memcpy(n->value, &kv, sizeof(key_value));
    return n;
}

constexpr int N_A = 0;

struct node_proto {
    int key = N_A;
    int level = 0;
    int lkey = N_A;
    int rkey = N_A;
};

static void construct_tree(tree_map *t, const node_proto *protos_p, size_t protos_n)
{
    std::map<int, tree_node *> nodes;
    for (size_t i = 0; i < protos_n; i++) {
        int key = protos_p[i].key;
        nodes.emplace(key, new_node(t, key_value(key, 0), protos_p[i].level));
    }
    for (size_t i = 0; i < protos_n; i++) {
        auto *n = nodes[protos_p[i].key];
        if (protos_p[i].lkey > 0) {
            n->child[0] = (tree_node *) nodes[protos_p[i].lkey];
        }
        if (protos_p[i].rkey) {
            n->child[1] = (tree_node *) nodes[protos_p[i].rkey];
        }
    }
    t->root = nodes[protos_p[0].key];
    t->size = protos_n;
}

static void construct_tree_check(tree_map *t, const node_proto *protos_p, size_t protos_n)
{
    construct_tree(t, protos_p, protos_n);
    ASSERT_TRUE(check_tree(*t));
}

TEST(tree_map, checks)
{
    memmgr_ctx mmc{};
    {
        // Left child violated
        tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
        node_proto protos[] = {
                {500, 1, 250, 750},
                {250, 1},
                {750, 1},
        };
        construct_tree(&t, protos, array_sizeof(protos));
        EXPECT_EQ(t.root, tree_map_check(&t));
    }
    {
        // Left child violated
        tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
        node_proto protos[] = {
                {500, 3, 250, 750},
                {250, 1},
                {750, 2, 700, 800},
                {700, 1},
                {800, 1},
        };
        construct_tree(&t, protos, array_sizeof(protos));
        EXPECT_EQ(t.root, tree_map_check(&t));
    }
    {
        // Left child violated
        tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
        node_proto protos[] = {
                {500, 1, 250, 750},
                {250, 1},
                {750, 1},
        };
        construct_tree(&t, protos, array_sizeof(protos));
        EXPECT_EQ(t.root, tree_map_check(&t));
    }
    {
        // Right child
        tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
        node_proto protos[] = {
                {500, 3, 250, 750},
                {250, 2, 200, 300},
                {200, 1},
                {300, 1},
                {750, 1},
        };
        construct_tree(&t, protos, array_sizeof(protos));
        EXPECT_EQ(t.root, tree_map_check(&t));
    }
    {
        // Right grand child
        tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
        node_proto protos[] = {
                {500, 2, 250, 750},
                {250, 1},
                {750, 2, 700, 850},
                {700, 1},
                {850, 2, 840, 860},
                {840, 1},
                {860, 1},
        };
        construct_tree(&t, protos, array_sizeof(protos));
        EXPECT_EQ(t.root, tree_map_check(&t));
    }
}

TEST(tree_map, put_get)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
    EXPECT_EQ(0, tree_map_size(&t));

    tree_map_put_v(&t, key_value(300, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 1));
    ASSERT_TRUE(check_tree(t));

    tree_map_put_v(&t, key_value(100, 2));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(100, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 1));
    ASSERT_TRUE(check_tree(t));

    tree_map_put_v(&t, key_value(200, 3));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(200, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(100, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 1));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(3, tree_map_size(&t));

    EXPECT_EQ(2, ((key_value *) tree_map_get_v(&t, 100))->v);
    EXPECT_EQ(3, ((key_value *) tree_map_get_v(&t, 200))->v);
    EXPECT_EQ(1, ((key_value *) tree_map_get_v(&t, 300))->v);

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, put_skew_split)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1, N_A, 300},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_put_v(&t, key_value(200, 0));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t));

    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t, 0));

    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_h1_c0)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_del_v(&t, 250);
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(2, tree_map_size(&t));

    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_h1_c1)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_del_v(&t, 750);
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(2, tree_map_size(&t));

    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_h1_root)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_del_v(&t, 500);
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(2, tree_map_size(&t));

    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_c0_case3_case1_1)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 4, 250, 750},
            {250, 3, 150, 350},
            {150, 2, 140, 160},
            {140, 1},
            {160, 1},
            {350, 2, 340, 360},
            {340, 1},
            {360, 1},
            {750, 4, 650, 850},
            {650, 3, 600, 700},
            {600, 2, 590, 610},
            {590, 1},
            {610, 1},
            {700, 2, 690, 710},
            {690, 1},
            {710, 1},
            {850, 3, 800, 900},
            {800, 2, 790, 810},
            {790, 1},
            {810, 1},
            {900, 2, 890, 910},
            {890, 1},
            {910, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    const int rm_key = 160;
    tree_map_del_v(&t, rm_key);
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) != nullptr);
        }
    }

    EXPECT_EQ(22, tree_map_size(&t));

    EXPECT_EQ(4, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 0));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t, 0, 0));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 0, 1));
    EXPECT_EQ(650, GET_TREE_NODE_KEY(&t, 0, 1));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 1, 0));
    EXPECT_EQ(600, GET_TREE_NODE_KEY(&t, 0, 1, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 1, 1));
    EXPECT_EQ(700, GET_TREE_NODE_KEY(&t, 0, 1, 1));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(850, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_c0_case3_case1_2)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 4, 250, 750},
            {250, 3, 150, 350},
            {150, 2, 140, 160},
            {140, 1},
            {160, 1},
            {350, 2, 340, 360},
            {340, 1},
            {360, 1},
            {750, 4, 650, 850},
            {650, 3, 600, 700},
            {600, 2, 590, 610},
            {590, 1},
            {610, 1},
            {700, 3, 690, 710},
            {690, 2, 685, 695},
            {685, 1},
            {695, 1},
            {710, 2, 705, 715},
            {705, 1},
            {715, 1},
            {850, 3, 800, 900},
            {800, 2, 790, 810},
            {790, 1},
            {810, 1},
            {900, 2, 890, 910},
            {890, 1},
            {910, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    const int rm_key = 160;
    tree_map_del_v(&t, rm_key);
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) != nullptr);
        }
    }

    EXPECT_EQ(26, tree_map_size(&t));

    EXPECT_EQ(4, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(650, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 0));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t, 0, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 1));
    EXPECT_EQ(600, GET_TREE_NODE_KEY(&t, 0, 1));
    EXPECT_EQ(4, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 1, 0));
    EXPECT_EQ(700, GET_TREE_NODE_KEY(&t, 1, 0));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 1, 1));
    EXPECT_EQ(850, GET_TREE_NODE_KEY(&t, 1, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_c1_case1)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 4, 250, 750},
            {250, 3, 150, 350},
            {150, 2, 140, 160},
            {140, 1},
            {160, 1},
            {350, 3, 300, 400},
            {300, 2, 290, 310},
            {290, 1},
            {310, 1},
            {400, 2, 390, 410},
            {390, 1},
            {410, 1},
            {750, 3, 650, 850},
            {650, 2, 640, 660},
            {640, 1},
            {660, 1},
            {850, 2, 840, 860},
            {840, 1},
            {860, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    const int rm_key = 860;
    tree_map_del_v(&t, rm_key);
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) != nullptr);
        }
    }

    EXPECT_EQ(18, tree_map_size(&t));

    EXPECT_EQ(4, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(350, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 0));
    EXPECT_EQ(150, GET_TREE_NODE_KEY(&t, 0, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 0, 1));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 1));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1, 0));
    EXPECT_EQ(400, GET_TREE_NODE_KEY(&t, 1, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1, 1));
    EXPECT_EQ(650, GET_TREE_NODE_KEY(&t, 1, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_c1_case2)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 4, 250, 750},
            {250, 3, 150, 350},
            {150, 2, 140, 160},
            {140, 1},
            {160, 1},
            {350, 2, 340, 360},
            {340, 1},
            {360, 1},
            {750, 3, 650, 850},
            {650, 2, 640, 660},
            {640, 1},
            {660, 1},
            {850, 2, 840, 860},
            {840, 1},
            {860, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    const int rm_key = 860;
    tree_map_del_v(&t, rm_key);
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) != nullptr);
        }
    }

    EXPECT_EQ(14, tree_map_size(&t));

    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(150, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 1));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1, 0));
    EXPECT_EQ(350, GET_TREE_NODE_KEY(&t, 1, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1, 1));
    EXPECT_EQ(650, GET_TREE_NODE_KEY(&t, 1, 1));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1, 1, 0));
    EXPECT_EQ(640, GET_TREE_NODE_KEY(&t, 1, 1, 0));
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1, 1, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1, 1, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_c0_middle)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 4, 250, 750},
            {250, 3, 150, 350},
            {150, 2, 140, 160},
            {140, 1},
            {160, 1},
            {350, 2, 340, 360},
            {340, 1},
            {360, 1},
            {750, 3, 650, 850},
            {650, 2, 640, 660},
            {640, 1},
            {660, 1},
            {850, 2, 840, 860},
            {840, 1},
            {860, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    const int rm_key = 250;
    tree_map_del_v(&t, rm_key);
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) != nullptr);
        }
    }

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, del_c1_middle)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 4, 250, 750},
            {250, 3, 150, 350},
            {150, 2, 140, 160},
            {140, 1},
            {160, 1},
            {350, 2, 340, 360},
            {340, 1},
            {360, 1},
            {750, 3, 650, 850},
            {650, 2, 640, 660},
            {640, 1},
            {660, 1},
            {850, 2, 840, 860},
            {840, 1},
            {860, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    const int rm_key = 750;
    tree_map_del_v(&t, rm_key);
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get_v(&t, protos[i].key) != nullptr);
        }
    }

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, cloning)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map c = tree_map_clone(&t);
    tree_map_destroy(&t);

    EXPECT_EQ(7, tree_map_size(&c));
    for (size_t i = 0; i < array_sizeof(protos); i++) {
        EXPECT_TRUE(tree_map_get_v(&c, protos[i].key) != nullptr);
    }

    tree_map_destroy(&c);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, replace)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_put_v(&t, key_value(200, 5));
    tree_map_put_v(&t, key_value(500, 6));
    tree_map_put_v(&t, key_value(750, 8));

    EXPECT_EQ(7, tree_map_size(&t));
    EXPECT_EQ(6, ((key_value *) tree_map_get_v(&t, 500))->v);
    EXPECT_EQ(0, ((key_value *) tree_map_get_v(&t, 250))->v);
    EXPECT_EQ(5, ((key_value *) tree_map_get_v(&t, 200))->v);
    EXPECT_EQ(0, ((key_value *) tree_map_get_v(&t, 300))->v);
    EXPECT_EQ(8, ((key_value *) tree_map_get_v(&t, 750))->v);
    EXPECT_EQ(0, ((key_value *) tree_map_get_v(&t, 700))->v);
    EXPECT_EQ(0, ((key_value *) tree_map_get_v(&t, 800))->v);

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

static long generate_trees(std::vector<tree_node *> &result, int level)
{
    if (level == 1) {
        auto *n = (tree_node *) memmgr_allocate_clear(nullptr, sizeof(tree_node));
        n->level = level;
        n->child[0] = nullptr;
        n->child[1] = nullptr;
        result.push_back(n);
        return 1;
    }

    long count = generate_trees(result, level - 1);
    long size = result.size();
    long prev_level_first = result.size() - count;

    result.reserve(count * count * (count + 1));

    for (long i = prev_level_first; i < size; i++) {
        for (long j = prev_level_first; j < size; j++) {
            auto *n = (tree_node *) memmgr_allocate_clear(nullptr, sizeof(tree_node));
            n->level = level;
            n->child[0] = result[i];
            n->child[1] = result[j];
            result.push_back(n);
        }
    }

    for (long i = prev_level_first; i < size; i++) {
        for (long j = prev_level_first; j < size; j++) {
            for (long k = prev_level_first; k < size; k++) {
                auto *n = (tree_node *) memmgr_allocate_clear(nullptr, sizeof(tree_node));
                auto *m = (tree_node *) memmgr_allocate_clear(nullptr, sizeof(tree_node));
                n->level = level;
                m->level = level;
                n->child[0] = result[i];
                n->child[1] = m;
                m->child[0] = result[j];
                m->child[1] = result[k];
                result.push_back(n);
            }
        }
    }

    return result.size() - size;
}

static void release_trees(std::vector<tree_node *> &all_trees)
{
    for (auto *n : all_trees) {
        memmgr_release(nullptr, n);
    }
    all_trees.clear();
}

static tree_node *build_tree(tree_map *t, const tree_node *node)
{
    if (node == nullptr) {
        return (tree_node *) &t->empty;
    }

    size_t n_bytes = sizeof(tree_node) + sizeof(key_value);
    auto *n = (tree_node *) memmgr_allocate_dirty(t->mmc, n_bytes);
    memcpy(n, node, n_bytes);
    n->child[0] = (tree_node *) build_tree(t, (tree_node *) node->child[0]);
    *((key_value *) n->value) = key_value(t->size++, 0);
    n->child[1] = (tree_node *) build_tree(t, (tree_node *) node->child[1]);

    return n;
}

TEST(tree_map, exhaustive_delete)
{
    std::vector<tree_node *> all_trees;
    long count = generate_trees(all_trees, 4);
    long first = all_trees.size() - count;

    for (long i = first; i < all_trees.size(); i++) {
        memmgr_ctx mmc{};
        tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));
        t.root = build_tree(&t, all_trees[i]);

        // print_tree(t, std::cout);

        for (long k = 0; k < tree_map_size(&t); k++) {
            tree_map c = tree_map_clone(&t);
            ASSERT_TRUE(check_tree(c));
            tree_map_del_v(&c, k);
            ASSERT_TRUE(check_tree(c));
            tree_map_destroy(&c);
        }

        tree_map_destroy(&t);
        EXPECT_EQ(0, mmc.n_allocs);
    }

    release_trees(all_trees);
}

TEST(tree_map, min_max_key)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    auto min_key = tree_map_min_key(&t);
    EXPECT_EQ(200, ((key_value *) min_key)->k);

    auto max_key = tree_map_max_key(&t);
    EXPECT_EQ(800, ((key_value *) max_key)->k);

    tree_map_destroy(&t);
    EXPECT_EQ(0, mmc.n_allocs);
}

TEST(tree_map, iterator_empty)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    tree_map_iter iter;
    EXPECT_EQ(nullptr, tree_map_begin(&t, &iter, true));
    EXPECT_EQ(nullptr, tree_map_begin(&t, &iter, false));
}

TEST(tree_map, iterator_fwd)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1, N_A, 210},
            {210, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    long keys[array_sizeof(protos)] = {};
    size_t i = 0;

    tree_map_iter iter;
    for (void *v = tree_map_begin(&t, &iter, true); v != nullptr; v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(8, i);
    EXPECT_EQ(200, keys[0]);
    EXPECT_EQ(210, keys[1]);
    EXPECT_EQ(250, keys[2]);
    EXPECT_EQ(300, keys[3]);
    EXPECT_EQ(500, keys[4]);
    EXPECT_EQ(700, keys[5]);
    EXPECT_EQ(750, keys[6]);
    EXPECT_EQ(800, keys[7]);
}

TEST(tree_map, iterator_back)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1, N_A, 210},
            {210, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    long keys[array_sizeof(protos)] = {};
    size_t i = 0;

    tree_map_iter iter;
    for (void *v = tree_map_begin(&t, &iter, false); v != nullptr; v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(8, i);
    EXPECT_EQ(800, keys[0]);
    EXPECT_EQ(750, keys[1]);
    EXPECT_EQ(700, keys[2]);
    EXPECT_EQ(500, keys[3]);
    EXPECT_EQ(300, keys[4]);
    EXPECT_EQ(250, keys[5]);
    EXPECT_EQ(210, keys[6]);
    EXPECT_EQ(200, keys[7]);
}

TEST(tree_map, iterator_at_fwd)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1, N_A, 210},
            {210, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    long keys[array_sizeof(protos)] = {};
    size_t i = 0;

    tree_map_iter iter;
    for (void *v = tree_map_begin_at_v(&t, &iter, true, 250); v != nullptr;
            v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(6, i);
    EXPECT_EQ(250, keys[0]);
    EXPECT_EQ(300, keys[1]);
    EXPECT_EQ(500, keys[2]);
    EXPECT_EQ(700, keys[3]);
    EXPECT_EQ(750, keys[4]);
    EXPECT_EQ(800, keys[5]);

    i = 0;
    for (void *v = tree_map_begin_at_v(&t, &iter, true, 500); v != nullptr;
            v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(4, i);
    EXPECT_EQ(500, keys[0]);
    EXPECT_EQ(700, keys[1]);
    EXPECT_EQ(750, keys[2]);
    EXPECT_EQ(800, keys[3]);

    i = 0;
    for (void *v = tree_map_begin_at_v(&t, &iter, true, 720); v != nullptr;
            v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(2, i);
    EXPECT_EQ(750, keys[0]);
    EXPECT_EQ(800, keys[1]);
}

TEST(tree_map, iterator_at_back)
{
    memmgr_ctx mmc{};
    tree_map t = tree_map_new(skey_compare, &mmc, sizeof(key_value));

    // clang-format off
    node_proto protos[] = {
            {500, 3, 250, 750},
            {250, 2, 200, 300},
            {200, 1, N_A, 210},
            {210, 1},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    // clang-format on
    construct_tree_check(&t, protos, array_sizeof(protos));

    long keys[array_sizeof(protos)] = {};
    size_t i = 0;

    tree_map_iter iter;
    for (void *v = tree_map_begin_at_v(&t, &iter, false, 250); v != nullptr;
            v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(3, i);
    EXPECT_EQ(250, keys[0]);
    EXPECT_EQ(210, keys[1]);
    EXPECT_EQ(200, keys[2]);

    i = 0;
    for (void *v = tree_map_begin_at_v(&t, &iter, false, 500); v != nullptr;
            v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(5, i);
    EXPECT_EQ(500, keys[0]);
    EXPECT_EQ(300, keys[1]);
    EXPECT_EQ(250, keys[2]);
    EXPECT_EQ(210, keys[3]);
    EXPECT_EQ(200, keys[4]);

    i = 0;
    for (void *v = tree_map_begin_at_v(&t, &iter, false, 720); v != nullptr;
            v = tree_map_iter_next(&iter)) {
        keys[i++] = ((key_value *) v)->k;
    }

    EXPECT_EQ(6, i);
    EXPECT_EQ(700, keys[0]);
    EXPECT_EQ(500, keys[1]);
    EXPECT_EQ(300, keys[2]);
    EXPECT_EQ(250, keys[3]);
    EXPECT_EQ(210, keys[4]);
    EXPECT_EQ(200, keys[5]);
}
