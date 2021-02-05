#include "owl/treemap.h"

static tree_node empty;

void treemap_init(void)
{
    empty.child[0] = &empty;
    empty.child[1] = &empty;
}

bool treemap_is_null(tree_node *n)
{
    return n == &empty;
}

treemap treemap_new(node_memmgr *nmm, void *ctx, size_t value_size)
{
    return (treemap){
            .nmm = nmm,
            .nmm_ctx = ctx,
            .node_size = sizeof(tree_node) + value_size,
            .size = 0,
            .root = &empty,
    };
}

inline static void treemap_init_node(tree_node *n, const lkey *key)
{
    n->key = *key;
    n->level = 1;
    n->child[0] = &empty;
    n->child[1] = &empty;
}

static void treemap_destroy_rec(node_memmgr *nmm, void *nmm_ctx, tree_node *node)
{
    if (node != &empty) {
        treemap_destroy_rec(nmm, nmm_ctx, node->child[0]);
        treemap_destroy_rec(nmm, nmm_ctx, node->child[1]);
        nmm->release(nmm_ctx, node);
    }
}

void treemap_destroy(treemap *t)
{
    treemap_destroy_rec(t->nmm, t->nmm_ctx, t->root);
}

typedef struct {
    treemap *tree;
    const lkey *key;
    tree_node *res;
} put_st;

static tree_node *treemap_put_rec(put_st *state, tree_node *node)
{
    if (node == &empty) {
        tree_node *p = state->tree->nmm->allocatez(state->tree->nmm_ctx, state->tree->node_size);
        treemap_init_node(p, state->key);
        state->tree->size++;
        state->res = p;
        return p;
    }

    int d = lkey_compare(state->key, &node->key);
    if (d == 0) {
        state->res = node;
        return node;
    }

    int branch = d > 0;
    tree_node *p = node;
    tree_node *current_child = p->child[branch];
    tree_node *c = (p->child[branch] = treemap_put_rec(state, current_child));
    if (c == current_child) {
        // Fast return: do not touch other nodes (possibly not in cache)
        return node;
    }

    if (branch) {
        // Right child: test if we need to increment level first
        if (p->level < c->level) {
            p->level++;
        }
    } else if (p->level == c->level) {
        // Rotate right
        p->child[0] = c->child[1];
        c->child[1] = p;
        p = c;
    }

    // Check if we have 3 nodes of the same level of the right
    if (p->level == p->child[1]->child[1]->level) {
        c = p->child[1];
        p->child[1] = c->child[0];
        c->child[0] = p;
        c->level++;
        p = c;
    }

    return p;
}

void *treemap_put(treemap *t, lkey key)
{
    put_st state = {.tree = t, .key = &key};
    t->root = treemap_put_rec(&state, t->root);
    return state.res->value;
}

void *treemap_get(treemap *t, lkey key)
{
    tree_node *node = t->root;
    while (node != &empty) {
        int d = lkey_compare(&key, &node->key);
        if (d == 0) {
            return node->value;
        }
        node = node->child[d > 0];
    }
    return NULL;
}

typedef struct {
    treemap *tree;
    const lkey *key;
    tree_node *removed;
} delete_st;

static tree_node *treemap_delete_rec(delete_st *state, tree_node *node)
{
    return NULL;
    /*
    if (node == &empty) {
        return node;
    }

    int d = lkey_compare(state->key, &node->key);
    int branch = d > 0;
    if (d == 0) {
        if (node->child[0] == &empty) {
            state->removed = node;
            return node->child[1]; // Replacement of this node
        }

        // Remove rightmost node of the left child instead. Find its key.
        tree_node *r = node->child[0];
        while (r->child[1] != &empty) {
            r = r->child[1];
        }
        state->key = &r->key;

        // @branch correctly points to 0
        node->child[0] = treemap_delete_rec(state, node->child[0]);

        // Removed node replaces this node. This node becomes removed.
        tree_node *t = state->removed;
        t->level = node->level;
        t->child[0] = node->child[0];
        t->child[1] = node->child[1];
        state->removed = node;
        node = t;
    } else {
        tree_node *p = node;
        tree_node *current_child = p->child[branch];
        tree_node *c = (p->child[branch] = treemap_delete_rec(state, current_child));
        if (c == current_child) {
            // Fast return: do not touch other nodes (possibly not in cache)
            return node;
        }
    }

    // The only change can happen is that child's level is decreased by 1. If level becomes too
    // low, we need to decrement it and fix AA properties.
    if (!(node->child[branch]->level < node->level - 1)) {
        return node;
    }

    node->level--;

    // Restore AA properties at this node.
    tree_node *u, *v;
    if (branch == 0) {
        // Two cases are possible based on level of the right child. Pictures after level decrement.
        //
        // Case 1:
        // -------
        //
        //      A(n-1)
        //    /        \
        // B(n-2)     C(n)
        //           /    \
        //        D(n-1) E(n-1)
        //
        // Case 2:
        // -------
        //
        //      A(n-1)
        //    /        \
        // B(n-2)     C(n-1)
        //           /      \
        //        D(n-2)   E(n-?)
        //
        // In case 1, we need to move A to the left.
        // ... transformed into:
        //
        //          C(n)
        //        /      \
        //  u->A(n-1)   E(n-1)
        //    /      \
        // B(n-2)   D(n-1)
        //
        // Right grand child property can be violated at A. If so, we split A, skew C.
        // ... transformed into:
        //
        //          C(n-1)
        //        /        \
        //  u->A(n-1)    E(n-?)
        //    /      \
        // B(n-2)   D(n-2)
        //
        // Left child property can be violated at C. Skew C.

        u = node->child[0];
        if (u->level == u->child[1]->child[1]->level) {
            v = u->child[1];
            u->child[1] = v->child[0];
            v->child[0] = u;
            v->level++;
            node->child[0] = v;
        }

        u = node->child[0];
        if (u->level == node->level) {
            node->child[0] = u->child[1];
            u->child[1] = node;
            node = u;
        }

        // Right grand child property can be violated at A. If so, we split A, skew C.

        // Case (1) - level after decrement shown:
        //
        //    A[n-1] <-- node
        //   /      \
        // B[n-2]   C[n-1] <-- r
        //
        // When we decrement level of A, the only property that can be violated is
        // right grandchild level. Apply split.
        if (r->level == node->level) {
            if (r->child[1]->level == node->level) {
                node->child[1] = r->child[0];
                r->child[0] = node;
                node = r;
                node->level++;
            }
            return node;
        }

        // Case (2) - level after decrement shown:
        //
        //    A[n-1] <-- node
        //   /      \
        // B[n-2]   C[n] <-- r
        //         /    \
        //       D[n-1] E[n-1]
        //
        // Move A to the left.
        node->child[1] = r->child[0];
        r->child[0] = node;
        node = r;

        // Now we have:
        //        C[n] <-- node
        //      /
        //    A[n-1] <-- l
        //   /      \
        // B[n-2]   D[n-1] <-- r
        //
        // Check if right grandchild property is violated at node A. If so, then we apply split
        // on A. After this, left child property is violated at node C. Apply skew.

        tree_node *l = node->child[0];
        if (l->child[1]->child[1]->level == l->level) {
            // Split
            r = l->child[1];
            l->child[1] = r->child[0];
            r->child[0] = l;
            r->level++;
            node->child[0] = r;

            // Now we have:
            //        C[n] <-- node
            //      /
            //    D[n] <-- l
            //   /    \
            // A[n-1] F[n-1]
            //
            // C.child[1] = E, not shown on the picture.

            // Skew
            l = node->child[0];
            node->child[0] = l->child[1];
            l->child[1] = node;
            node = l;
        }
        return node;
    } else {
        return node;
    }
*/
}

bool treemap_delete(treemap *t, lkey key)
{
    delete_st state = {.tree = t, .key = &key};
    t->root = treemap_delete_rec(&state, t->root);
    if (!state.removed) {
        return false;
    }
    t->nmm->release(t->nmm_ctx, state.removed);
    t->size--;
    return true;
}
