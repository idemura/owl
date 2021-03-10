#include "foundation/tree_map.h"

#include <stdio.h>

#define SKEY_OF_NODE(n) SKEY_OF((n)->value)

tree_map tree_map_new(skey_compare_fn compare_keys, memmgr_ctx *mmc, size_t value_size)
{
    tree_map t = {
            .compare_keys = compare_keys,
            .value_size = pad_size_l(value_size),
            .mmc = mmc,
    };
    t.empty.child[0] = &t.empty;
    t.empty.child[1] = &t.empty;
    t.root = &t.empty;
    return t;
}

tree_node *tree_map_clone_rec(tree_map *c, const tree_map *t, const tree_node *node)
{
    if (node == &t->empty) {
        return &c->empty;
    }

    size_t n_bytes = sizeof(tree_node) + c->value_size;
    tree_node *n = memmgr_allocate_dirty(c->mmc, n_bytes);
    memcpy(n, node, n_bytes);
    n->child[0] = tree_map_clone_rec(c, t, node->child[0]);
    n->child[1] = tree_map_clone_rec(c, t, node->child[1]);

    return n;
}

tree_map tree_map_clone(const tree_map *t)
{
    tree_map c = *t;
    c.root = tree_map_clone_rec(&c, t, t->root);
    return c;
}

static const tree_node *tree_map_check_rec(const tree_node *node)
{
    if (node->level == 0) {
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

static void tree_map_destroy_rec(memmgr_ctx *mmc, tree_node *node)
{
    if (node->level != 0) {
        tree_map_destroy_rec(mmc, node->child[0]);
        tree_map_destroy_rec(mmc, node->child[1]);
        memmgr_release(mmc, node);
    }
}

void tree_map_destroy(tree_map *t)
{
    tree_map_destroy_rec(t->mmc, t->root);
    *t = (tree_map){0};
}

// Rotate. Moves child @b up and makes node @n its `(1 - b)` child.
inline static tree_node *tree_map_rotate(tree_node *n, int b)
{
    tree_node *c = n->child[b];
    n->child[b] = c->child[1 - b];
    c->child[1 - b] = n;
    return c;
}

static tree_node *tree_map_put_rec(tree_map *t, skey_t key, tree_node *node)
{
    if (node->level == 0) {
        tree_node *n = memmgr_allocate_dirty(t->mmc, sizeof(tree_node) + t->value_size);
        n->child[0] = &t->empty;
        n->child[1] = &t->empty;
        n->level = 1;
        memcpy(n->value, key.ptr, t->value_size);
        t->size++;
        return n;
    }

    long d = t->compare_keys(key, SKEY_OF_NODE(node));
    if (d == 0) {
        tree_node *n = node;
        memcpy(n->value, key.ptr, t->value_size);
        return node;
    }

    int branch = d > 0;
    node->child[branch] = tree_map_put_rec(t, key, node->child[branch]);

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

void tree_map_put(tree_map *t, skey_t key_value)
{
    t->root = tree_map_put_rec(t, key_value, t->root);
}

ATTR_NO_INLINE
static tree_node *tree_map_find_node(tree_map *t, skey_t key)
{
    tree_node *node = t->root;
    while (node->level != 0) {
        long d = t->compare_keys(key, SKEY_OF_NODE(node));
        if (d == 0) {
            return node;
        }
        node = node->child[d > 0];
    }
    return NULL;
}

void *tree_map_get(tree_map *t, skey_t key)
{
    tree_node *node = tree_map_find_node(t, key);
    return node ? node->value : NULL;
}

static tree_node *tree_map_left_max_key(tree_node *node)
{
    tree_node *r = node->child[0];
    while (r->child[1]->level != 0) {
        r = r->child[1];
    }
    return r;
}

static tree_node *tree_map_fix_node_delete(tree_node *node, int b)
{
    // The only change that can happen is that child's level is decreased by 1. If level becomes
    // too low, we need to decrement it and fix AA properties.
    if (!(node->child[b]->level < node->level - 1)) {
        return node;
    }

    node->level--;

    // Restore AA properties at this node.
    tree_node *x = node;
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
        //
        //  _A(-1)_
        // /       \
        //       _C(-1)_
        //      /       \
        //   D(-2)     E(-2)
        //

        tree_node *y = node->child[1];
        if (x->level == y->level) {
            // Case 2 and 3
            if (y->child[1]->level == y->level) {
                // Case 2: split
                x = tree_map_rotate(x, 1);
                x->level++;
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
        //
        //       _A(-1)_
        //      /       \
        //  _B(-1)_    C(-2)
        // /       \
        //        D(-1)
        //
        // Case 2:
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

        tree_node *y = x->child[1];
        if (y->child[0]->level == y->level) {
            // Case 1:
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

static tree_node *tree_map_del_rec(tree_map *t, skey_t key, tree_node *node)
{
    if (node->level == 0) {
        return node;
    }

    long d = t->compare_keys(key, SKEY_OF_NODE(node));
    int branch = d > 0;
    if (d == 0) {
        if (node->child[0]->level == 0) {
            return node->child[1]; // Replacement of this node
        }

        tree_node *left_max = tree_map_left_max_key(node);
        skey_t new_key = SKEY_OF_NODE(left_max);

        // @branch correctly points to 0
        node->child[0] = tree_map_del_rec(t, new_key, node->child[0]);

        // Left max node replaces this one (which is deleted). Copy
        *left_max = *node;
        node = left_max;
    } else {
        node->child[branch] = tree_map_del_rec(t, key, node->child[branch]);
    }

    return tree_map_fix_node_delete(node, branch);
}

void tree_map_del(tree_map *t, skey_t key)
{
    tree_node *node = tree_map_find_node(t, key);
    if (node != NULL) {
        t->root = tree_map_del_rec(t, key, t->root);
        memmgr_release(t->mmc, node);
        t->size--;
    }
}

const tree_node *tree_map_path(tree_map *t, int path_len, const int *path)
{
    const tree_node *p = t->root;
    for (int i = 0; i < path_len; i++) {
        p = p->child[path[i]];
    }
    return (const tree_node *) p;
}

void *tree_map_min_key(tree_map *t)
{
    tree_node *node = t->root;
    if (node->level == 0) {
        return NULL;
    }

    while (node->child[0]->level != 0) {
        node = node->child[0];
    }
    return node->value;
}

void *tree_map_max_key(tree_map *t)
{
    tree_node *node = t->root;
    if (node->level == 0) {
        return NULL;
    }

    while (node->child[1]->level != 0) {
        node = node->child[1];
    }
    return node->value;
}

void *tree_map_begin(tree_map *t, tree_map_iter *iter, bool fwd)
{
    iter->top = 0;
    iter->empty = &t->empty;
    iter->dir = fwd ? 0 : 1;

    // Our stack contains only child[dir] of the path.
    tree_node *n = t->root;
    while (n != iter->empty) {
        iter->stack[iter->top] = n;
        iter->top++;
        n = n->child[iter->dir];
    }

    return tree_map_iter_next(iter);
}

void *tree_map_begin_at(tree_map *t, tree_map_iter *iter, bool fwd, skey_t key)
{
    iter->top = 0;
    iter->empty = &t->empty;
    iter->dir = fwd ? 0 : 1;

    tree_node *n = t->root;
    while (n->level != 0) {
        long d = t->compare_keys(key, SKEY_OF_NODE(n));
        if (d == 0 || (d > 0) == iter->dir) {
            iter->stack[iter->top] = n;
            iter->top++;
            if (d == 0) {
                break;
            }
            n = n->child[iter->dir];
        } else {
            // If we want to start to the right of this node, it whole subtree is not eligible.
            n = n->child[1 - iter->dir];
        }
    }

    return tree_map_iter_next(iter);
}

void *tree_map_iter_next(tree_map_iter *iter)
{
    if (iter->top == 0) {
        return NULL;
    }

    iter->top--;
    tree_node *top = iter->stack[iter->top];

    // Next is the leftmost child of the right subtree or next up the stack.
    if (top->child[1 - iter->dir] != iter->empty) {
        tree_node *n = top->child[1 - iter->dir];
        do {
            iter->stack[iter->top] = n;
            iter->top++;
            n = n->child[iter->dir];
        } while (n != iter->empty);
    }

    return top->value;
}
