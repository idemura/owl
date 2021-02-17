#include "owl/tree_map.h"

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <map>
#include <string>

#define GET_TREE_NODE_LEVEL(t, ...) tree_map_path_va(t, __VA_ARGS__)->link.level
#define GET_TREE_NODE_KEY(t, ...) tree_map_path_va(t, __VA_ARGS__)->key.nk

static long skey_compare(const skey_t *a, const skey_t *b)
{
    return a->nk - b->nk;
}

static std::string to_string(skey_t key)
{
    return key.sk ? std::string{key.sk} : std::to_string(key.nk);
}

static void print_child(std::ostream &os, const tree_node *c, const char *name)
{
    os << "child " << name << ": ";
    if (c->link.level == 0) {
        os << "null";
    } else {
        os << "key=" << to_string(c->key) << " level=" << c->link.level;
    }
}

static bool check_tree(const tree_map &t, std::ostream &os)
{
    const tree_node *e = tree_map_check(&t);
    if (e) {
        os << "AA property violated:\n";
        os << "at node key=" << to_string(e->key) << " level=" << e->link.level << "\n";

        os << "  ";
        print_child(os, (tree_node *) e->link.child[0], "0");
        os << "\n";

        os << "  ";
        print_child(os, (tree_node *) e->link.child[1], "1");
        os << "\n";

        if (e->link.child[1]->level != 0) {
            os << "  ";
            print_child(os, (tree_node *) e->link.child[1]->child[1], "1-1");
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
    if ((const tree_link *) node == &t.empty) {
        os << "(null)\n";
        return;
    }
    os << to_string(node->key) << " level=" << node->link.level << "\n";
    if (node->link.child[0] == &t.empty && node->link.child[1] == &t.empty) {
        return;
    }
    print_tree_rec(t, os, (const tree_node *) node->link.child[0], indent + 1);
    print_tree_rec(t, os, (const tree_node *) node->link.child[1], indent + 1);
}

static void print_tree(const tree_map &t, std::ostream &os)
{
    if ((const tree_link *) t.root == &t.empty) {
        os << "empty\n";
    } else {
        print_tree_rec(t, os, t.root, 0);
    }
}

static tree_node *new_node(tree_map *t, skey_t key, int level)
{
    const auto *mm = get_memmgr_for_test();
    auto n = (tree_node *) mm->allocate_clear(t->mm_ctx, sizeof(tree_node) + sizeof(int));
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
    int level = 0;
    long lkey = N_A;
    long rkey = N_A;
};

static void construct_tree(tree_map *t, const node_proto *protos_p, size_t protos_n)
{
    std::map<long, tree_node *> nodes;
    for (size_t i = 0; i < protos_n; i++) {
        long nk = protos_p[i].key;
        nodes.emplace(nk, new_node(t, skey_number(nk), protos_p[i].level));
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
}

static void construct_tree_check(tree_map *t, const node_proto *protos_p, size_t protos_n)
{
    construct_tree(t, protos_p, protos_n);
    ASSERT_TRUE(check_tree(*t));
}

TEST(tree_map, checks)
{
    mm_test_ctx mm_ctx{};
    {
        // Left child violated
        tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);
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
        tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);
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
        tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);
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
        tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);
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
        tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);
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
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, sizeof(int));
    EXPECT_EQ(0, tree_map_size(&t));

    *(int *) tree_map_put(&t, skey_number(300)) = 1;
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 1));
    ASSERT_TRUE(check_tree(t));

    *(int *) tree_map_put(&t, skey_number(100)) = 2;
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(100, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(0, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 1));
    ASSERT_TRUE(check_tree(t));

    *(int *) tree_map_put(&t, skey_number(200)) = 3;
    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(200, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(100, GET_TREE_NODE_KEY(&t, 0));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(300, GET_TREE_NODE_KEY(&t, 1));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(3, tree_map_size(&t));

    EXPECT_EQ(2, *(int *) tree_map_get(&t, skey_number(100)));
    EXPECT_EQ(3, *(int *) tree_map_get(&t, skey_number(200)));
    EXPECT_EQ(1, *(int *) tree_map_get(&t, skey_number(300)));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, put_skew_split)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1, N_A, 300},
            {300, 1},
            {750, 2, 700, 800},
            {700, 1},
            {800, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_put(&t, skey_number(200));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(3, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t));

    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 0));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t, 0));

    EXPECT_EQ(2, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_h1_c0)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_del(&t, skey_number(250));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(2, tree_map_size(&t));

    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_h1_c1)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_del(&t, skey_number(750));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(2, tree_map_size(&t));

    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(500, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_h1_root)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

    node_proto protos[] = {
            {500, 2, 250, 750},
            {250, 1},
            {750, 1},
    };
    construct_tree_check(&t, protos, array_sizeof(protos));

    tree_map_del(&t, skey_number(500));
    ASSERT_TRUE(check_tree(t));

    EXPECT_EQ(2, tree_map_size(&t));

    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t));
    EXPECT_EQ(250, GET_TREE_NODE_KEY(&t));
    EXPECT_EQ(1, GET_TREE_NODE_LEVEL(&t, 1));
    EXPECT_EQ(750, GET_TREE_NODE_KEY(&t, 1));

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_c0_case3_case1_1)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    const long rm_key = 160;
    tree_map_del(&t, skey_number(rm_key));
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get(&t, k) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get(&t, k) != nullptr);
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
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_c0_case3_case1_2)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    const long rm_key = 160;
    tree_map_del(&t, skey_number(rm_key));
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get(&t, k) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get(&t, k) != nullptr);
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
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_c1_case1)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    const long rm_key = 860;
    tree_map_del(&t, skey_number(rm_key));
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get(&t, k) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get(&t, k) != nullptr);
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
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_c1_case2)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    const long rm_key = 860;
    tree_map_del(&t, skey_number(rm_key));
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get(&t, k) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get(&t, k) != nullptr);
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
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_c0_middle)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    const long rm_key = 250;
    tree_map_del(&t, skey_number(rm_key));
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get(&t, k) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get(&t, k) != nullptr);
        }
    }

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, del_c1_middle)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    const long rm_key = 750;
    tree_map_del(&t, skey_number(rm_key));
    ASSERT_TRUE(check_tree(t));

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        if (protos[i].key == rm_key) {
            EXPECT_TRUE(tree_map_get(&t, k) == nullptr);
        } else {
            EXPECT_TRUE(tree_map_get(&t, k) != nullptr);
        }
    }

    tree_map_destroy(&t);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

TEST(tree_map, cloning)
{
    mm_test_ctx mm_ctx{};
    tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);

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

    for (size_t i = 0; i < array_sizeof(protos); i++) {
        auto k = skey_number(protos[i].key);
        EXPECT_TRUE(tree_map_get(&c, k) != nullptr);
    }

    tree_map_destroy(&c);
    EXPECT_EQ(0, mm_ctx.n_allocs);
}

static long generate_trees(std::vector<tree_node *> &result, int level)
{
    const auto *mm = get_memmgr_for_test();

    if (level == 1) {
        auto *n = (tree_node *) mm->allocate_clear(nullptr, sizeof(tree_node));
        n->link.level = level;
        n->link.child[0] = nullptr;
        n->link.child[1] = nullptr;
        result.push_back(n);
        return 1;
    }

    long count = generate_trees(result, level - 1);
    long size = result.size();
    long prev_level_first = result.size() - count;

    result.reserve(count * count * (count + 1));

    for (long i = prev_level_first; i < size; i++) {
        for (long j = prev_level_first; j < size; j++) {
            tree_node *n = (tree_node *) mm->allocate_clear(nullptr, sizeof(tree_node));
            n->link.level = level;
            n->link.child[0] = (tree_link *) result[i];
            n->link.child[1] = (tree_link *) result[j];
            result.push_back(n);
        }
    }

    for (long i = prev_level_first; i < size; i++) {
        for (long j = prev_level_first; j < size; j++) {
            for (long k = prev_level_first; k < size; k++) {
                tree_node *n = (tree_node *) mm->allocate_clear(nullptr, sizeof(tree_node));
                tree_node *m = (tree_node *) mm->allocate_clear(nullptr, sizeof(tree_node));
                n->link.level = level;
                m->link.level = level;
                n->link.child[0] = (tree_link *) result[i];
                n->link.child[1] = (tree_link *) m;
                m->link.child[0] = (tree_link *) result[j];
                m->link.child[1] = (tree_link *) result[k];
                result.push_back(n);
            }
        }
    }

    return result.size() - size;
}

static void release_trees(std::vector<tree_node *> &all_trees)
{
    const auto *mm = get_memmgr_for_test();
    for (auto *n : all_trees) {
        mm->release(nullptr, n);
    }
    all_trees.clear();
}

static tree_node *build_tree(tree_map *t, const tree_node *node)
{
    if (node == nullptr) {
        return (tree_node *) &t->empty;
    }

    auto *n = (tree_node *) t->mm->allocate_dirty(t->mm_ctx, sizeof(tree_node));
    *n = *node;
    n->link.child[0] = (tree_link *) build_tree(t, (tree_node *) node->link.child[0]);
    n->key.nk = t->size++;
    n->link.child[1] = (tree_link *) build_tree(t, (tree_node *) node->link.child[1]);
    return n;
}

TEST(tree_map, exhaustive_delete)
{
    const auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<tree_node *> all_trees;
    long count = generate_trees(all_trees, 4);
    long first = all_trees.size() - count;

    for (long i = first; i < all_trees.size(); i++) {
        mm_test_ctx mm_ctx{};
        tree_map t = tree_map_new(skey_compare, get_memmgr_for_test(), &mm_ctx, 0);
        t.root = build_tree(&t, all_trees[i]);

        for (long k = 0; k < tree_map_size(&t); k++) {
            tree_map c = tree_map_clone(&t);
            ASSERT_TRUE(check_tree(c));
            tree_map_del(&c, skey_number(k));
            ASSERT_TRUE(check_tree(c));
            tree_map_destroy(&c);
        }

        tree_map_destroy(&t);
        EXPECT_EQ(0, mm_ctx.n_allocs);
    }

    release_trees(all_trees);

    std::chrono::duration<double, std::micro> duration =
            std::chrono::high_resolution_clock::now() - start_time;

    std::cout << "Duration: " << duration.count() << " us\n";
}
