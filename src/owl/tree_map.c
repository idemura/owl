#include "owl/tree_map.h"

#include <assert.h>
#include <stdio.h>

typedef struct {
    const tree_key *key;
    tree_link *result;
} rec_state;

tree_map tree_map_new(
        compare_keys_fn compare_keys, node_memmgr *nmm, void *nmm_ctx, size_t value_size)
{
    // clang-format off
    tree_map t = {
            .compare_keys = compare_keys,
            .nmm = nmm,
            .nmm_ctx = nmm_ctx,
            .node_size = sizeof(tree_node) + value_size
    };
    // clang-format on
    t.empty.child[0] = &t.empty;
    t.empty.child[1] = &t.empty;
    t.root = (tree_node *) &t.empty;
    return t;
}

static const tree_link *tree_map_check_rec(const tree_link *node)
{
    if (node->level == 0) {
        return NULL;
    }

    const tree_link *t;

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
    return (tree_node *) tree_map_check_rec((tree_link *) t->root);
}

static void tree_map_destroy_rec(node_memmgr *nmm, void *nmm_ctx, tree_link *node)
{
    if (node->level != 0) {
        tree_map_destroy_rec(nmm, nmm_ctx, node->child[0]);
        tree_map_destroy_rec(nmm, nmm_ctx, node->child[1]);
        nmm->release(nmm_ctx, (tree_node *) node);
    }
}

void tree_map_destroy(tree_map *t)
{
    tree_map_destroy_rec(t->nmm, t->nmm_ctx, (tree_link *) t->root);
}

// Rotate. Moves child @b up and makes node @n its `(1 - b)` child.
inline static tree_link *tree_map_rotate(tree_link *n, int b)
{
    tree_link *c = n->child[b];
    n->child[b] = c->child[1 - b];
    c->child[1 - b] = n;
    return c;
}

static tree_link *tree_map_put_rec(tree_map *t, rec_state *state, tree_link *node)
{
    if (node->level == 0) {
        tree_node *n = t->nmm->allocatez(t->nmm_ctx, t->node_size);
        n->link.child[0] = &t->empty;
        n->link.child[1] = &t->empty;
        n->link.level = 1;
        n->key = *state->key;
        t->size++;
        state->result = (tree_link *) n;
        return state->result;
    }

    int d = t->compare_keys(state->key, &((tree_node *) node)->key);
    if (d == 0) {
        state->result = node;
        return node;
    }

    int branch = d > 0;
    node->child[branch] = tree_map_put_rec(t, state, node->child[branch]);

    int level_diff = node->level - node->child[branch]->level;
    if (level_diff) {
        assert(level_diff == 1);
        return node;
    }

    if (branch == 0) {
        node = tree_map_rotate(node, 0);
    }
    // Check if we child 1-1 have the same level
    if (node->level == node->child[1]->child[1]->level) {
        node = tree_map_rotate(node, 1);
        node->level++;
    }

    return node;
}

void *tree_map_put(tree_map *t, tree_key key)
{
    rec_state state = {.key = &key};
    t->root = (tree_node *) tree_map_put_rec(t, &state, (tree_link *) t->root);
    return ((tree_node *) state.result)->value;
}

void *tree_map_get(tree_map *t, tree_key key)
{
    tree_node *node = t->root;
    while (node->link.level != 0) {
        int d = t->compare_keys(&key, &node->key);
        if (d == 0) {
            return node->value;
        }
        node = (tree_node *) node->link.child[d > 0];
    }
    return NULL;
}

static tree_link *tree_map_left_max_key(tree_link *node)
{
    tree_link *r = node->child[0];
    while (r->child[1]->level != 0) {
        r = r->child[1];
    }
    return r;
}

static tree_link *tree_map_fix_node_delete(tree_link *node, unsigned b)
{
    // The only change that can happen is that child's level is decreased by 1. If level becomes
    // too low, we need to decrement it and fix AA properties.
    if (!(node->child[b]->level < node->level - 1)) {
        return node;
    }

    node->level--;

    // Restore AA properties at this node.
    tree_link *x = node, *y = node->child[1 - b];
    if (b == 0) {
        printf("fix child 0\n");
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
                printf("case 2\n");
                x = tree_map_rotate(x, 1);
            } else {
                printf("case 3\n");
            }
            return x;
        }

        printf("case 1\n");
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
            printf("complex case 1\n");
            x->child[0] = tree_map_rotate(x->child[0], 1);
            x->child[0]->level++;
            x = tree_map_rotate(x, 0);
        }

        return x;
    } else {
        printf("fix child 1\n");
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

static tree_link *tree_map_del_rec(tree_map *t, rec_state *state, tree_link *node)
{
    if (node->level == 0) {
        return node;
    }

    int d = t->compare_keys(state->key, &((tree_node *) node)->key);
    int branch = d > 0;
    if (d == 0) {
        if (node->child[0]->level == 0) {
            state->result = node;
            return node->child[1]; // Replacement of this node
        }

        printf("look for left max key\n");
        tree_node *left_max = (tree_node *) tree_map_left_max_key((tree_link *) node);
        state->key = &left_max->key; // New key to delete

        // @branch correctly points to 0
        node->child[0] = tree_map_del_rec(t, state, node->child[0]);

        // Removed node replaces this node. This node becomes removed.
        tree_link *t = state->result;
        *t = *node;
        state->result = node;
        node = t;
    } else {
        node->child[branch] = tree_map_del_rec(t, state, node->child[branch]);
    }

    return tree_map_fix_node_delete(node, branch);
}

bool tree_map_del(tree_map *t, tree_key key)
{
    rec_state state = {.key = &key};
    t->root = (tree_node *) tree_map_del_rec(t, &state, (tree_link *) t->root);
    if (!state.result) {
        return false;
    }
    t->nmm->release(t->nmm_ctx, (tree_node *) state.result);
    t->size--;
    return true;
}

const tree_node *tree_map_path(tree_map *t, int path_len, const int *path)
{
    const tree_link *p = (tree_link *) t->root;
    for (int i = 0; i < path_len; i++) {
        p = p->child[path[i]];
    }
    return (const tree_node *) p;
}
