#include "owl/tree_map.h"

#include <assert.h>

typedef struct {
    tree_map *tree;
    const tree_key *key;
    tree_node *result;
} rec_state;

static tree_node empty = {.child = {[0] = &empty, [1] = &empty}};

tree_node *tree_map_empty(void)
{
    return &empty;
}

bool tree_map_is_null(const tree_node *n)
{
    return n == &empty;
}

tree_map tree_map_new(node_memmgr *nmm, void *ctx, size_t value_size)
{
    return (tree_map){
            .nmm = nmm,
            .nmm_ctx = ctx,
            .node_size = sizeof(tree_node) + value_size,
            .size = 0,
            .root = &empty,
    };
}

static const tree_node *tree_map_check_rec(const tree_node *node)
{
    if (node == &empty) {
        return NULL;
    }

    const tree_node *t;

    if ((t = tree_map_check_rec(node->child[0]))) {
        return t;
    }
    if ((t = tree_map_check_rec(node->child[1]))) {
        return t;
    }

    // Child 0 level is one less
    if (node->child[0]->level != node->level - 1) {
        return node;
    }
    // Child 1 level is either one less or equal
    if (node->child[1]->level == node->level - 1) {
        return NULL;
    }
    if (node->child[1]->level != node->level) {
        return node;
    }
    // If equal, child 1-1 level is one less
    if (node->child[1]->child[1]->level != node->level - 1) {
        return node;
    }
    return NULL;
}

const tree_node *tree_map_check(const tree_map *t)
{
    return tree_map_check_rec(t->root);
}

static void tree_map_destroy_rec(node_memmgr *nmm, void *nmm_ctx, tree_node *node)
{
    if (node != &empty) {
        tree_map_destroy_rec(nmm, nmm_ctx, node->child[0]);
        tree_map_destroy_rec(nmm, nmm_ctx, node->child[1]);
        nmm->release(nmm_ctx, node);
    }
}

void tree_map_destroy(tree_map *t)
{
    tree_map_destroy_rec(t->nmm, t->nmm_ctx, t->root);
}

// Rotate. Moves child @b up and makes node @n its `(1 - b)` child.
inline static tree_node *tree_map_rotate(tree_node *n, int b)
{
    tree_node *c = n->child[b];
    n->child[b] = c->child[1 - b];
    c->child[1 - b] = n;
    return c;
}

static tree_node *tree_map_put_rec(rec_state *state, tree_node *node)
{
    if (node == &empty) {
        tree_node *p = state->tree->nmm->allocatez(state->tree->nmm_ctx, state->tree->node_size);
        p->key = *state->key;
        p->level = 1;
        p->child[0] = &empty;
        p->child[1] = &empty;
        state->tree->size++;
        state->result = p;
        return p;
    }

    int d = tree_key_compare(state->key, &node->key);
    if (d == 0) {
        state->result = node;
        return node;
    }

    int branch = d > 0;
    tree_node *x = node;
    x->child[branch] = tree_map_put_rec(state, x->child[branch]);

    if (x->child[branch]->level != x->level) {
        assert(x->child[branch]->level == x->level - 1);
        return x;
    }

    if (branch == 0) {
        x = tree_map_rotate(x, 0);
    }
    // Check if we child 1-1 have the same level
    if (x->level == x->child[1]->child[1]->level) {
        x = tree_map_rotate(x, 1);
        x->level++;
    }

    return x;
}

void *tree_map_put(tree_map *t, tree_key key)
{
    rec_state state = {.tree = t, .key = &key};
    t->root = tree_map_put_rec(&state, t->root);
    return state.result->value;
}

void *tree_map_get(tree_map *t, tree_key key)
{
    tree_node *node = t->root;
    while (node != &empty) {
        int d = tree_key_compare(&key, &node->key);
        if (d == 0) {
            return node->value;
        }
        node = node->child[d > 0];
    }
    return NULL;
}

static tree_node *tree_map_left_max_key(tree_node *node)
{
    tree_node *r = node->child[0];
    while (r->child[1] != &empty) {
        r = r->child[1];
    }
    assert(r->child[0] == &empty);
    return r;
}

static tree_node *tree_map_fix_node_delete(tree_node *node, unsigned b)
{
    // The only change that can happen is that child's level is decreased by 1. If level becomes
    // too low, we need to decrement it and fix AA properties.
    if (!(node->child[b]->level < node->level - 1)) {
        return node;
    }

    node->level--;

    // Restore AA properties at this node.
    tree_node *x = node, *y = node->child[1 - b];
    if (b == 0) {
        // Three cases are possible.
        //
        // Case 1:
        //
        //  _A(-1)_
        // /       \
        //       ___C___
        //      /       \
        //   D(-1)     E(-1)
        //
        // Case 2:
        //
        //  _A(-1)_
        // /       \
        //       _C(-1)_
        //      /       \
        //   D(-2)     E(-1)
        //
        // Case 3:
        // -------
        //
        //  _A(-1)_
        // /       \
        //       _C(-1)_
        //      /       \
        //   D(-2)     E(-2)
        //

        if (x->level == y->level) {
            // Case 2 and 3
            if (y->child[1]->level == y->level) {
                // Case 2: split
                x = tree_map_rotate(x, 1);
            }
            return x;
        }

        assert(x->level == x->child[1]->level - 1);
        // Case 1: move child[1] up
        x = tree_map_rotate(x, 1);

        //       ___C___
        //      /       \
        //  _A(-1)_    E(-1)
        // /       \
        //        D(-1)
        //       /     \
        //            F(-?)
        //
        // Check if node A violates right grandchild property: split, skew.
        y = x->child[0];
        if (y->level == y->child[1]->child[1]->level) {
            x->child[0] = tree_map_rotate(x->child[0], 1);
            x->child[0]->level++;
            x = tree_map_rotate(x, 0);
        }

        return x;
    } else {
        // Two cases are possible based on level of the right child of left child.
        // Pictures before level decrement.
        //
        // Case 1:
        // -------
        //
        //       _A(-1)_
        //      /       \
        //  _B(-1)_    C(-2)
        // /       \
        //        D(-1)
        //
        // Case 2:
        // -------
        //
        //       _A(-1)_
        //      /       \
        //  _B(-1)_    C(-2)
        // /       \
        //        D(-2)
        //

        assert(x->level == x->child[0]->level);
        // Always fix left child. Case 2 is fixed after this.
        x = tree_map_rotate(x, 0);

        if (x->child[1]->child[0]->level == x->child[1]->level) {
            // Case 1:
            // -------
            //
            //  _B(-1)_
            // /       \
            //       _A(-1)_
            //      /       \
            //   D(-1)     C(-2)
            //
            x->child[1] = tree_map_rotate(x->child[1], 0);
            x = tree_map_rotate(x, 1);
            x->level++;
        }

        return x;
    }
}

static tree_node *tree_map_del_rec(rec_state *state, tree_node *node)
{
    if (node == &empty) {
        return node;
    }

    int d = tree_key_compare(state->key, &node->key);
    int branch = d > 0;
    if (d == 0) {
        if (node->child[0] == &empty) {
            state->result = node;
            return node->child[1]; // Replacement of this node
        }

        tree_node *left_max = tree_map_left_max_key(node);
        state->key = &left_max->key; // New key to delete

        // @branch correctly points to 0
        node->child[0] = tree_map_del_rec(state, node->child[0]);

        // Removed node replaces this node. This node becomes removed.
        tree_node *t = state->result;
        t->level = node->level;
        t->child[0] = node->child[0];
        t->child[1] = node->child[1];
        state->result = node;
        node = t;
    } else {
        node->child[branch] = tree_map_del_rec(state, node->child[branch]);
    }

    return tree_map_fix_node_delete(node, branch);
}

bool tree_map_del(tree_map *t, tree_key key)
{
    rec_state state = {.tree = t, .key = &key};
    t->root = tree_map_del_rec(&state, t->root);
    if (!state.result) {
        return false;
    }
    t->nmm->release(t->nmm_ctx, state.result);
    t->size--;
    return true;
}

const tree_node *tree_map_path(tree_map *t, int path_len, ...)
{
    va_list va;
    va_start(va, path_len);
    const tree_node *p = t->root;
    for (int i = 0; i < path_len; i++) {
        p = p->child[va_arg(va, int)];
    }
    va_end(va);
    return p;
}
